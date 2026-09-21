#include "test_framework.h"

#include "../src/domain/chat.h"
#include "../src/graphics/canvas.h"
#include "../src/hal/async_worker.h"
#include "../src/mtproto/telegram_client.h"
#include "../src/mtproto/tl_codec.h"
#include "../src/ui/chat_list_screen.h"
#include "../src/ui/screen_navigator.h"
#include "mocks/mock_network_transport.h"

#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

namespace {

const char* TEST_SESSION_CHAT = "/tmp/test_session_chat.dat";

void cleanupChatSession() {
    std::remove(TEST_SESSION_CHAT);
}

std::vector<std::uint8_t> makeDialogsPayload(int count) {
    mtproto::TlWriter writer;
    writer.write_int32(0x105);
    writer.write_int32(count);
    for (int i = 0; i < count; ++i) {
        writer.write_int64(100 + i);
        writer.write_string("Conversation " + std::to_string(i + 1));
        writer.write_int32(i % 2);
    }
    return writer.data();
}

} // namespace

TEST(chat_list_screen_renders_loading_state_during_fetch) {
    cleanupChatSession();
    MockNetworkTransport transport;
    transport.setResponse(makeDialogsPayload(2));
    mtproto::TelegramClient client(transport, TEST_SESSION_CHAT);
    ScreenNavigator navigator;
    hal::AsyncWorker worker;
    ChatListScreen screen(client, navigator, &worker);

    screen.onEnter();

    // Before draining callbacks, the screen should be in LOADING state
    Canvas canvas;
    screen.render(canvas);

    // Verify loading indicator rendered
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(30, 80, 500, 150)));

    // Clean up worker
    worker.drainUiCallbacks();
    worker.stop();
    cleanupChatSession();
}

TEST(chat_list_screen_populates_real_dialogs_from_client) {
    cleanupChatSession();
    MockNetworkTransport transport;
    transport.setResponse(makeDialogsPayload(3));
    mtproto::TelegramClient client(transport, TEST_SESSION_CHAT);
    ScreenNavigator navigator;
    ChatListScreen screen(client, navigator);

    screen.onEnter();
    Canvas canvas;
    screen.render(canvas);

    // Header & dialog rows rendered
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(0, 0, 600, 30)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(10, 40, 590, 85)));

    InputEvent downEvent{KeyCode::KEY_DOWN, true};
    screen.handleInput(downEvent);

    InputEvent enterEvent{KeyCode::KEY_ENTER, true};
    screen.handleInput(enterEvent);
    cleanupChatSession();
}

TEST(chat_list_screen_renders_empty_state_when_no_dialogs) {
    cleanupChatSession();
    MockNetworkTransport transport;
    transport.setResponse(makeDialogsPayload(0));
    mtproto::TelegramClient client(transport, TEST_SESSION_CHAT);
    ScreenNavigator navigator;
    ChatListScreen screen(client, navigator);

    screen.onEnter();
    Canvas canvas;
    screen.render(canvas);

    // Header rendered
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(0, 0, 600, 30)));
    // Empty state message rendered around y=100
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(30, 80, 500, 150)));
    // But no chat row separator lines
    ASSERT_FALSE(canvas.hasNonWhitePixel(BoundingBox(10, 46, 590, 47)));
    cleanupChatSession();
}

TEST(chat_list_screen_shows_error_banner_on_fetch_failure) {
    cleanupChatSession();
    MockNetworkTransport transport;
    transport.setSuccess(false);
    mtproto::TelegramClient client(transport, TEST_SESSION_CHAT);
    ScreenNavigator navigator;
    ChatListScreen screen(client, navigator);

    screen.onEnter();
    Canvas canvas;
    screen.render(canvas);

    // Error message/banner rendered around y=100 or toast
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(30, 80, 560, 200)));
    cleanupChatSession();
}
