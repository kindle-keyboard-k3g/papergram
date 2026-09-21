#include "test_framework.h"

#include "../src/domain/message.h"
#include "../src/domain/value_objects.h"
#include "../src/graphics/canvas.h"
#include "../src/hal/async_worker.h"
#include "../src/mtproto/telegram_client.h"
#include "../src/mtproto/tl_codec.h"
#include "../src/ui/conversation_screen.h"
#include "../src/ui/screen_navigator.h"
#include "mocks/mock_network_transport.h"

#include <cstdio>
#include <vector>

namespace {

const char* TEST_SESSION_CONV = "/tmp/test_session_conv.dat";

void cleanupConvSession() {
    std::remove(TEST_SESSION_CONV);
}

std::vector<std::uint8_t> makeHistoryPayload(int count) {
    mtproto::TlWriter writer;
    writer.write_int32(0x206);
    writer.write_int32(count);
    for (int i = 0; i < count; ++i) {
        writer.write_int64(1000 + i);
        writer.write_int64(42);
        writer.write_string(i % 2 == 0 ? "Alice" : "You");
        writer.write_string("Message " + std::to_string(i + 1));
        writer.write_int64(1700000000 + i * 60);
        writer.write_int32(i % 2);
    }
    return writer.data();
}

} // namespace

TEST(conversation_screen_wraps_messages_at_fifty_eight_characters) {
    ConversationScreen screen(ChatId(7), "Kindle");
    const std::string long_text(120U, 'x');
    screen.addMessage(Message(MessageId(1), ChatId(7), "Alice", long_text,
                              0, false));

    ASSERT_TRUE(screen.wrappedLines().size() >= 3U);
    for (const std::string& line : screen.wrappedLines()) {
        ASSERT_TRUE(line.size() <= 58U);
    }
}

TEST(conversation_screen_accepts_keyboard_text_and_submits) {
    ConversationScreen screen(ChatId(7), "Kindle");
    screen.handleInput(InputEvent{KeyCode::KEY_H, true});
    screen.handleInput(InputEvent{KeyCode::KEY_I, true});
    ASSERT_STR_EQ("hi", screen.inputText());

    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});
    ASSERT_STR_EQ("hi", screen.consumeSubmittedMessage());
    ASSERT_STR_EQ("", screen.inputText());
}

TEST(conversation_screen_renders_incoming_and_outgoing_messages) {
    ConversationScreen screen(ChatId(7), "Kindle");
    screen.addMessage(Message(MessageId(1), ChatId(7), "Alice", "hello", 0, false));
    screen.addMessage(Message(MessageId(2), ChatId(7), "Me", "sent", 0, true));
    Canvas canvas;

    screen.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(0, 48, 599, 799)));
}

TEST(conversation_screen_loads_history_on_enter) {
    cleanupConvSession();
    MockNetworkTransport transport;
    transport.setResponse(makeHistoryPayload(2));
    mtproto::TelegramClient client(transport, TEST_SESSION_CONV);
    ScreenNavigator navigator;
    ConversationScreen screen(client, navigator);
    screen.setChat(ChatId(42), "Tech Chat");

    screen.onEnter();
    Canvas canvas;
    screen.render(canvas);

    // Verify messages are rendered
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(0, 48, 599, 700)));
    cleanupConvSession();
}

TEST(conversation_screen_renders_empty_state_when_no_messages) {
    cleanupConvSession();
    MockNetworkTransport transport;
    transport.setResponse(makeHistoryPayload(0));
    mtproto::TelegramClient client(transport, TEST_SESSION_CONV);
    ScreenNavigator navigator;
    ConversationScreen screen(client, navigator);
    screen.setChat(ChatId(42), "Empty Chat");

    screen.onEnter();
    Canvas canvas;
    screen.render(canvas);

    // Empty state message rendered around y=100
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(30, 80, 500, 150)));
    cleanupConvSession();
}

TEST(conversation_screen_sends_message_via_client_optimistically) {
    cleanupConvSession();
    MockNetworkTransport transport;
    transport.setResponse(makeHistoryPayload(1));
    mtproto::TelegramClient client(transport, TEST_SESSION_CONV);
    ScreenNavigator navigator;
    ConversationScreen screen(client, navigator);
    screen.setChat(ChatId(42), "Chat");
    screen.onEnter();

    // Type "hello"
    screen.handleInput(InputEvent{KeyCode::KEY_H, true});
    screen.handleInput(InputEvent{KeyCode::KEY_E, true});
    screen.handleInput(InputEvent{KeyCode::KEY_L, true});
    screen.handleInput(InputEvent{KeyCode::KEY_L, true});
    screen.handleInput(InputEvent{KeyCode::KEY_O, true});

    // Set canned response for sendMessage RPC
    mtproto::TlWriter sendResponse;
    sendResponse.write_int32(0x521);
    transport.setResponse(sendResponse.data());

    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    // Input buffer should be cleared
    ASSERT_STR_EQ("", screen.inputText());

    // Optimistically rendered
    Canvas canvas;
    screen.render(canvas);
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(120, 48, 560, 200)));

    cleanupConvSession();
}

TEST(conversation_screen_shows_error_toast_when_send_fails) {
    cleanupConvSession();
    MockNetworkTransport transport;
    // History loads ok
    transport.setResponse(makeHistoryPayload(1));
    mtproto::TelegramClient client(transport, TEST_SESSION_CONV);
    ScreenNavigator navigator;
    ConversationScreen screen(client, navigator);
    screen.setChat(ChatId(42), "Chat");
    screen.onEnter();

    screen.handleInput(InputEvent{KeyCode::KEY_H, true});
    screen.handleInput(InputEvent{KeyCode::KEY_I, true});

    // Now transport fails on sendMessage
    transport.setSuccess(false);
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    Canvas canvas;
    screen.render(canvas);

    // Error toast rendered in bottom area (around y=580..680)
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(40, 580, 560, 680)));

    cleanupConvSession();
}
