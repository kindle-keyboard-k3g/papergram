#include "domain/chat.h"
#include "domain/message.h"
#include "domain/value_objects.h"
#include "graphics/canvas.h"
#include "graphics/dark_to_white_cleaner.h"
#include "graphics/diff_tracker.h"
#include "graphics/idle_refresh_scheduler.h"
#include "graphics/refresh_strategy.h"
#include "hal/async_worker.h"
#include "hal/eink_controller_mxc.h"
#include "hal/fallback_devices.h"
#include "hal/frame_buffer_fb0.h"
#include "hal/http_transport.h"
#include "hal/input_device_evdev.h"
#include "mtproto/telegram_client.h"
#include "ui/chat_list_screen.h"
#include "ui/conversation_screen.h"
#include "ui/login_screen.h"
#include "ui/popup_manager.h"
#include "ui/screen_navigator.h"
#include "ui/screensaver_screen.h"

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>

namespace {
volatile std::sig_atomic_t g_running = 1;

void handleSignal(int) {
    g_running = 0;
}

std::uint64_t monotonicTimeMs() {
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

struct HardwareContext {
    std::unique_ptr<IFrameBuffer> fb;
    std::unique_ptr<IEinkController> eink;
    std::unique_ptr<IInputDevice> input;
};

HardwareContext initializeHardware() {
    HardwareContext ctx;
    try {
        ctx.fb = std::make_unique<FrameBufferLinuxFb0>();
        ctx.eink = std::make_unique<EinkControllerMxc>();
        ctx.input = std::make_unique<InputDeviceEvdev>();
        std::cout << "[Kindle Telegram] Hardware framebuffer and e-ink initialized." << std::endl;
        return ctx;
    } catch (const std::exception& ex) {
        std::cout << "[Kindle Telegram] Native e-ink unavailable (" << ex.what()
                  << "). Using host fallback." << std::endl;
        ctx.fb = std::make_unique<MemoryFrameBuffer>();
        ctx.eink = std::make_unique<DummyEinkController>();
        ctx.input = std::make_unique<StdinInputDevice>();
        std::cout << "[Kindle Telegram] [Host Mode] Framebuffer is written to /tmp/kindle_fb.ppm" << std::endl;
        std::cout << "[Kindle Telegram] [Host Mode] Controls: type keys, Enter: submit, Backspace: delete, ~: lock/unlock, Ctrl+D: exit" << std::endl;
        return ctx;
    }
}

void handlePowerToggle(ScreenNavigator& navigator) {
    if (navigator.isLocked()) {
        navigator.unlockScreen();
        return;
    }
    navigator.lockScreen();
}

void renderFrame(ScreenNavigator& navigator, ui::PopupManager& popups, Canvas& canvas, IFrameBuffer& fb) {
    navigator.render(canvas);
    popups.render(canvas);
    fb.copyFrom(canvas.backBuffer(), Canvas::BUFFER_SIZE);
    fb.flush();
}

struct RefreshPipeline {
    FullRefresh full_refresh;
    DarkToWhiteCleaner d2w_cleaner;
    BufferDiffTracker diff_tracker;
    IdleRefreshScheduler idle_scheduler;
    bool alt_pressed = false;

    explicit RefreshPipeline(IEinkController& controller)
        : full_refresh(controller),
          d2w_cleaner(controller),
          diff_tracker(),
          idle_scheduler(controller) {}
};

bool handleAltGhostbuster(const InputEvent& ev, RefreshPipeline& pipeline) {
    if (ev.code == KeyCode::KEY_ALT) {
        pipeline.alt_pressed = ev.pressed;
        return true;
    }
    if (pipeline.alt_pressed && ev.code == KeyCode::KEY_G && ev.pressed) {
        std::cout << "[Kindle Telegram] Alt+G Ghostbuster manual full refresh triggered." << std::endl;
        pipeline.full_refresh.forceFullRefresh();
        return true;
    }
    return false;
}

void presentFrame(ScreenNavigator& navigator, ui::PopupManager& popups, Canvas& canvas,
                  IFrameBuffer& fb, RefreshPipeline& pipeline, bool forceScreenChanged) {
    renderFrame(navigator, popups, canvas, fb);
    pipeline.diff_tracker.compare(canvas.frontBuffer(), canvas.backBuffer());

    if (!forceScreenChanged && !pipeline.diff_tracker.hasChanges()) {
        return;
    }

    RefreshResult result = pipeline.full_refresh.refresh(pipeline.diff_tracker.changed(), forceScreenChanged);
    if (result == RefreshResult::PartialDu && pipeline.diff_tracker.hasDarkToWhite()) {
        pipeline.d2w_cleaner.clean(pipeline.diff_tracker.darkToWhite());
    }
    canvas.swapBuffers();
    std::cout << "[Kindle Telegram] Screen updated -> /tmp/kindle_fb.ppm" << std::endl;
}

void processEvent(const InputEvent& ev, ScreenNavigator& navigator, ui::PopupManager& popups,
                  Canvas& canvas, HardwareContext& hw, RefreshPipeline& pipeline) {
    pipeline.idle_scheduler.noteActivity(monotonicTimeMs());
    if (handleAltGhostbuster(ev, pipeline)) {
        return;
    }
    if (!ev.pressed) {
        return;
    }
    if (ev.code == KeyCode::KEY_POWER) {
        handlePowerToggle(navigator);
        presentFrame(navigator, popups, canvas, *hw.fb, pipeline, true);
        return;
    }
    if (navigator.isLocked()) {
        return;
    }

    IScreen* prev_screen = navigator.currentScreen();
    navigator.handleInput(ev);
    bool screen_changed = (navigator.currentScreen() != prev_screen);
    presentFrame(navigator, popups, canvas, *hw.fb, pipeline, screen_changed);
}
}

int main() {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    std::cout << "[Kindle Telegram] Starting..." << std::endl;
    HardwareContext hw = initializeHardware();
    HttpTransport transport("149.154.167.50", 80);

    mtproto::TelegramClient client(transport);
    Canvas canvas;
    RefreshPipeline pipeline(*hw.eink);
    ui::PopupManager popups;

    ScreenNavigator navigator;
    navigator.setExitCallback([&]() {
        g_running = 0;
    });
    hal::AsyncWorker worker;
    auto login = std::make_unique<LoginScreen>(client, navigator, &worker);
    auto chat_list = std::make_unique<ChatListScreen>(client, navigator, &worker);
    auto conv = std::make_unique<ConversationScreen>(client, navigator, &worker);
    auto screensaver = std::make_unique<ScreensaverScreen>(&navigator);
    navigator.setScreens(std::move(login), std::move(chat_list), std::move(conv));
    navigator.setScreensaver(std::move(screensaver));

    if (client.isAuthorized()) navigator.showChatList();
    renderFrame(navigator, popups, canvas, *hw.fb);
    pipeline.full_refresh.forceFullRefresh();
    canvas.swapBuffers();
    pipeline.idle_scheduler.noteActivity(monotonicTimeMs());

    std::cout << "[Kindle Telegram] Ready. Initial frame saved to /tmp/kindle_fb.ppm" << std::endl;
    std::cout << "[Kindle Telegram] Entering main loop (listening for input)..." << std::endl;
    while (g_running) {
        InputEvent ev;
        bool has_event = hw.input->pollEvent(ev, 100);
        uint64_t now_ms = monotonicTimeMs();
        popups.update(ui::PopupTimestamp(std::chrono::milliseconds(now_ms)));

        std::size_t drained = worker.drainUiCallbacks();
        if (drained > 0) {
            presentFrame(navigator, popups, canvas, *hw.fb, pipeline, false);
        }

        if (has_event) {
            processEvent(ev, navigator, popups, canvas, hw, pipeline);
            continue;
        }
        if (popups.needsRedraw()) {
            presentFrame(navigator, popups, canvas, *hw.fb, pipeline, false);
            popups.acknowledgeRedraw();
            continue;
        }
        pipeline.idle_scheduler.tick(now_ms);

        if (hw.input->isClosed()) {
            std::cout << "[Kindle Telegram] Input stream closed. Exiting." << std::endl;
            break;
        }
    }

    worker.stop();
    std::cout << "[Kindle Telegram] Shutting down cleanly." << std::endl;
    return 0;
}
