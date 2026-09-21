#include "test_framework.h"
#include "mocks/mock_network_transport.h"
#include "../src/domain/chat.h"
#include "../src/domain/message.h"
#include "../src/domain/value_objects.h"
#include "../src/mtproto/telegram_client.h"
#include "../src/mtproto/tl_codec.h"

#include <cstdio>
#include <vector>

namespace {

const char* TEST_SESSION_PATH = "/tmp/test_session_rpc.dat";

void cleanupTestSession() {
    std::remove(TEST_SESSION_PATH);
}

} // namespace

TEST(telegram_client_decodes_dialogs_from_tl_response) {
    cleanupTestSession();
    MockNetworkTransport transport;
    mtproto::TelegramClient client(transport, TEST_SESSION_PATH);

    // Build TL response with 2 dialogs
    mtproto::TlWriter writer;
    writer.write_int32(0x105); // messages.dialogs constructor
    writer.write_int32(2);     // count

    // Dialog 1
    writer.write_int64(42);
    writer.write_string("Tech Chat");
    writer.write_int32(1);

    // Dialog 2
    writer.write_int64(99);
    writer.write_string("Kindle Dev");
    writer.write_int32(0);

    transport.setResponse(writer.data());

    ChatList chats;
    bool success = client.getDialogs(chats);

    ASSERT_TRUE(success);
    ASSERT_EQ(static_cast<std::size_t>(2), chats.count());
    ASSERT_EQ(static_cast<std::int64_t>(42), chats.at(0).id().value());
    ASSERT_STR_EQ("Tech Chat", chats.at(0).title().value());
    ASSERT_EQ(static_cast<std::uint32_t>(1), chats.at(0).unreadCount());

    ASSERT_EQ(static_cast<std::int64_t>(99), chats.at(1).id().value());
    ASSERT_STR_EQ("Kindle Dev", chats.at(1).title().value());
    ASSERT_EQ(static_cast<std::uint32_t>(0), chats.at(1).unreadCount());

    cleanupTestSession();
}

TEST(telegram_client_decodes_empty_dialogs) {
    cleanupTestSession();
    MockNetworkTransport transport;
    mtproto::TelegramClient client(transport, TEST_SESSION_PATH);

    // Build TL response with 0 dialogs
    mtproto::TlWriter writer;
    writer.write_int32(0x105);
    writer.write_int32(0);
    transport.setResponse(writer.data());

    ChatList chats;
    bool success = client.getDialogs(chats);

    ASSERT_TRUE(success);
    ASSERT_EQ(static_cast<std::size_t>(0), chats.count());

    cleanupTestSession();
}

TEST(telegram_client_returns_false_on_network_transport_failure) {
    cleanupTestSession();
    MockNetworkTransport transport;
    transport.setSuccess(false);
    mtproto::TelegramClient client(transport, TEST_SESSION_PATH);

    ChatList chats;
    ASSERT_FALSE(client.getDialogs(chats));

    MessageHistory history;
    ASSERT_FALSE(client.getHistory(ChatId(42), history));

    ASSERT_FALSE(client.sendMessage(ChatId(42), MessageText("Hello")));
    ASSERT_FALSE(client.requestAuthCode(PhoneNumber("+1234567890")));
    ASSERT_FALSE(client.signIn(PhoneNumber("+1234567890"), AuthCode("12345")));

    cleanupTestSession();
}

TEST(telegram_client_decodes_history_messages_from_tl_response) {
    cleanupTestSession();
    MockNetworkTransport transport;
    mtproto::TelegramClient client(transport, TEST_SESSION_PATH);

    // Build TL response with 2 messages
    mtproto::TlWriter writer;
    writer.write_int32(0x206); // messages.history constructor
    writer.write_int32(2);     // count

    // Message 1 (incoming)
    writer.write_int64(1001);
    writer.write_int64(42);
    writer.write_string("Alice");
    writer.write_string("Hey there");
    writer.write_int64(1700000000);
    writer.write_int32(0);

    // Message 2 (outgoing)
    writer.write_int64(1002);
    writer.write_int64(42);
    writer.write_string("You");
    writer.write_string("Hello!");
    writer.write_int64(1700000010);
    writer.write_int32(1);

    transport.setResponse(writer.data());

    MessageHistory history;
    bool success = client.getHistory(ChatId(42), history);

    ASSERT_TRUE(success);
    ASSERT_EQ(static_cast<std::size_t>(2), history.count());

    ASSERT_EQ(static_cast<std::int64_t>(1001), history.at(0).id().value());
    ASSERT_STR_EQ("Alice", history.at(0).sender());
    ASSERT_STR_EQ("Hey there", history.at(0).text().value());
    ASSERT_FALSE(history.at(0).isOutgoing());

    ASSERT_EQ(static_cast<std::int64_t>(1002), history.at(1).id().value());
    ASSERT_STR_EQ("You", history.at(1).sender());
    ASSERT_STR_EQ("Hello!", history.at(1).text().value());
    ASSERT_TRUE(history.at(1).isOutgoing());

    cleanupTestSession();
}

TEST(telegram_client_send_message_serializes_chat_and_text) {
    cleanupTestSession();
    MockNetworkTransport transport;
    mtproto::TelegramClient client(transport, TEST_SESSION_PATH);

    mtproto::TlWriter responseWriter;
    responseWriter.write_int32(0x521); // updateShortSentMessage constructor
    transport.setResponse(responseWriter.data());

    bool success = client.sendMessage(ChatId(42), MessageText("Testing message"));
    ASSERT_TRUE(success);

    // Verify serialized payload
    mtproto::TlReader reader(transport.lastPayload());
    ASSERT_EQ(0x520, reader.read_int32());
    ASSERT_EQ(static_cast<std::int64_t>(42), reader.read_int64());
    ASSERT_STR_EQ("Testing message", reader.read_string());

    cleanupTestSession();
}

TEST(telegram_client_auth_flow_parses_responses_and_persists_session) {
    cleanupTestSession();
    MockNetworkTransport transport;
    mtproto::TelegramClient client(transport, TEST_SESSION_PATH);

    // 1. sendCode / requestAuthCode
    mtproto::TlWriter codeWriter;
    codeWriter.write_int32(0x86b); // auth.sentCode constructor
    transport.setResponse(codeWriter.data());
    ASSERT_TRUE(client.sendCode(PhoneNumber("+1234567890")));

    // 2. signIn
    mtproto::TlWriter authWriter;
    authWriter.write_int32(0xcd05da1b); // auth.authorization
    std::vector<std::uint8_t> dummy_key(256, 0x42);
    authWriter.write_bytes(dummy_key);
    authWriter.write_int64(0x11223344ULL);
    authWriter.write_int64(0x55667788ULL);
    authWriter.write_int32(1);
    transport.setResponse(authWriter.data());

    ASSERT_TRUE(client.signIn(PhoneNumber("+1234567890"), AuthCode("12345")));
    ASSERT_TRUE(client.isAuthorized());

    cleanupTestSession();
}
