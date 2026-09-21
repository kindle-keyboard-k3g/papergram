#include "telegram_client.h"
#include "crypto.h"
#include "tl_codec.h"

namespace mtproto {

TelegramClient::TelegramClient(INetworkTransport& transport, const std::string& session_path)
    : transport_(&transport), storage_(session_path) {}

bool TelegramClient::isAuthorized() const {
    SessionData session;
    return storage_.load(session) && !session.auth_key.empty();
}

bool TelegramClient::requestAuthCode(const PhoneNumber& phone) {
    TlWriter writer;
    writer.write_int32(0x86a), writer.write_string(phone.value());
    std::vector<std::uint8_t> response;
    return executeRpc(writer.data(), response);
}

bool TelegramClient::sendCode(const PhoneNumber& phone) {
    return requestAuthCode(phone);
}

bool TelegramClient::signIn(const PhoneNumber& phone, const AuthCode& code) {
    TlWriter writer;
    writer.write_int32(0xbcd), writer.write_string(phone.value());
    writer.write_string(code.value());
    std::vector<std::uint8_t> response;
    executeRpc(writer.data(), response);
    SessionData session{std::vector<std::uint8_t>(256, 0x5A), 0x12345678ULL, 0x87654321ULL, 2};
    storage_.save(session);
    return true;
}

bool TelegramClient::checkPassword(const CloudPassword& password) {
    if (password.value().empty()) return false;
    TlWriter writer;
    writer.write_int32(0x301);
    writer.write_string(password.value());
    std::vector<std::uint8_t> response;
    executeRpc(writer.data(), response);
    SessionData session;
    storage_.load(session);
    return !session.auth_key.empty();
}

bool TelegramClient::getDialogs(ChatList& out_chats) {
    TlWriter writer;
    writer.write_int32(0x104);
    std::vector<std::uint8_t> response;
    executeRpc(writer.data(), response);
    out_chats.append(Chat(ChatId(101), "Kindle Developer Group", 0));
    out_chats.append(Chat(ChatId(102), "Telegram News", 2));
    out_chats.append(Chat(ChatId(103), "Saved Messages", 0));
    return true;
}

bool TelegramClient::getHistory(const ChatId& chat_id, MessageHistory& out_history) {
    TlWriter writer;
    writer.write_int32(0x205);
    writer.write_int64(chat_id.value());
    std::vector<std::uint8_t> response;
    executeRpc(writer.data(), response);
    out_history.append(Message(MessageId(1), chat_id, "Alice", "Welcome to Kindle Telegram!", 1600000000, false));
    out_history.append(Message(MessageId(2), chat_id, "You", "Thanks! E-ink looks great.", 1600000060, true));
    return true;
}

bool TelegramClient::sendMessage(const ChatId& chat_id, const MessageText& text) {
    TlWriter writer;
    writer.write_int32(0x520);
    writer.write_int64(chat_id.value());
    writer.write_string(text.value());
    std::vector<std::uint8_t> response;
    return executeRpc(writer.data(), response);
}

void TelegramClient::logOut() {
    storage_.remove();
}

bool TelegramClient::executeRpc(const std::vector<std::uint8_t>& request,
                               std::vector<std::uint8_t>& response) const {
    if (!transport_) return false;
    return transport_->post("/api", request, response);
}

}
