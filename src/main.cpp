#include "domain/chat.h"
#include "domain/message.h"
#include "domain/value_objects.h"
#include "graphics/canvas.h"
#include "graphics/dirty_tracker.h"
#include "graphics/refresh_strategy.h"
#include "hal/eink_controller_mxc.h"
#include "hal/fallback_devices.h"
#include "hal/frame_buffer_fb0.h"
#include "hal/http_transport.h"
#include "hal/input_device_evdev.h"
#include "mtproto/telegram_client.h"
#include "ui/chat_list_screen.h"
#include "ui/conversation_screen.h"
#include "ui/login_screen.h"
#include "ui/screen_navigator.h"

#include <csignal>
#include <iostream>
#include <memory>

namespace {
volatile std::sig_atomic_t g_running = 1;

void handleSignal(int) {
    g_running = 0;
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
        std::cout << "[Kindle Telegram] [Host Mode] Controls: type keys, Enter: submit, Backspace: delete, Ctrl+D: exit" << std::endl;
        return ctx;
    }
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
    DirtyTracker dirty_tracker;
    FullRefresh full_refresh(*hw.eink);
    TypingRefresh typing_refresh(*hw.eink);

    ScreenNavigator navigator;
    auto login = std::make_unique<LoginScreen>(client, navigator);
    auto chat_list = std::make_unique<ChatListScreen>(client, navigator);
    auto conv = std::make_unique<ConversationScreen>(client, navigator);
    navigator.setScreens(std::move(login), std::move(chat_list), std::move(conv));

    if (client.isAuthorized()) navigator.showChatList();
    navigator.render(canvas);
    hw.fb->flush();
    full_refresh.refresh(dirty_tracker);

    std::cout << "[Kindle Telegram] Ready. Initial frame saved to /tmp/kindle_fb.ppm" << std::endl;
    std::cout << "[Kindle Telegram] Entering main loop (listening for input)..." << std::endl;
    while (g_running) {
        InputEvent ev;
        if (hw.input->pollEvent(ev, 100)) {
            navigator.handleInput(ev);
            navigator.render(canvas);
            hw.fb->flush();
            typing_refresh.refresh(dirty_tracker);
            std::cout << "[Kindle Telegram] Screen updated -> /tmp/kindle_fb.ppm" << std::endl;
        }
        if (hw.input->isClosed()) {
            std::cout << "[Kindle Telegram] Input stream closed. Exiting." << std::endl;
            break;
        }
    }

    std::cout << "[Kindle Telegram] Shutting down cleanly." << std::endl;
    return 0;
}
