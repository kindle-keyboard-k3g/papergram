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

/**
 * @brief Stops the main loop after a termination signal.
 * @param signal_number Signal received from the operating system.
 */
void handleSignal(int signal_number) {
    (void)signal_number;
    g_running = 0;
}

/**
 * @brief Returns the current monotonic time in milliseconds.
 * @return Milliseconds elapsed on the process monotonic clock.
 */
std::uint64_t monotonicTimeMs() {
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

/**
 * @brief Owns the framebuffer, e-ink controller, and input device.
 */
struct HardwareContext {
    std::unique_ptr<IFrameBuffer> fb;
    std::unique_ptr<IEinkController> eink;
    std::unique_ptr<IInputDevice> input;
};

/**
 * @brief Initializes native Kindle hardware or host fallback devices.
 * @return Owning handles for the selected hardware implementation.
 */
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

/**
 * @brief Toggles the navigator between locked and unlocked states.
 * @param navigator Navigator whose lock state should change.
 */
void handlePowerToggle(ScreenNavigator& navigator) {
    if (navigator.isLocked()) {
        navigator.unlockScreen();
        return;
    }
    navigator.lockScreen();
}

/**
 * @brief Renders the navigator and popups, then flushes the framebuffer.
 * @param navigator Screen and overlay coordinator.
 * @param popups Popup overlay manager.
 * @param canvas Back-buffer canvas used for rendering.
 * @param fb Framebuffer receiving the rendered pixels.
 */
void renderFrame(ScreenNavigator& navigator, ui::PopupManager& popups, Canvas& canvas, IFrameBuffer& fb) {
    navigator.render(canvas);
    popups.render(canvas);
    fb.copyFrom(canvas.backBuffer(), Canvas::BUFFER_SIZE);
    fb.flush();
}

/**
 * @brief Groups e-ink refresh strategies and frame-diff state.
 */
struct RefreshPipeline {
    FullRefresh full_refresh;
    DarkToWhiteCleaner d2w_cleaner;
    BufferDiffTracker diff_tracker;
    IdleRefreshScheduler idle_scheduler;
    bool alt_pressed = false;

    /**
     * @brief Creates a refresh pipeline for an e-ink controller.
     * @param controller Controller used by full, cleaning, and idle refreshes.
     */
    explicit RefreshPipeline(IEinkController& controller)
        : full_refresh(controller),
          d2w_cleaner(controller),
          diff_tracker(),
          idle_scheduler(controller) {}
};

/**
 * @brief Handles Alt state and the manual Alt+G full-refresh shortcut.
 * @param ev Input event to inspect.
 * @param pipeline Refresh pipeline receiving a forced refresh when requested.
 * @return True when the event is consumed by ghostbuster handling.
 */
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

/**
 * @brief Renders a frame and performs the required e-ink refresh.
 * @param navigator Screen and overlay coordinator.
 * @param popups Popup overlay manager.
 * @param canvas Canvas containing the current frame.
 * @param fb Framebuffer receiving the frame.
 * @param pipeline Refresh strategies and frame-diff tracker.
 * @param forceScreenChanged Whether to force a full refresh for a transition.
 */
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

/**
 * @brief Processes one input event and presents any resulting frame.
 * @param ev Input event received from the input device.
 * @param navigator Screen and overlay coordinator.
 * @param popups Popup overlay manager.
 * @param canvas Canvas used for rendering.
 * @param hw Active hardware devices.
 * @param pipeline Refresh strategies and frame-diff tracker.
 */
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

/**
 * @brief Initializes Papergram and runs the input/render event loop.
 * @return Zero after a clean shutdown.
 */
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
