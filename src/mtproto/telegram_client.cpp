#include "telegram_client.h"
#include "crypto.h"
#include "tl_codec.h"

namespace mtproto {

namespace {

bool decodeDialogs(const std::vector<std::uint8_t>& data, ChatList& out_chats) {
    if (data.empty()) return false;
    try {
        TlReader reader(data);
        reader.read_int32(); // constructor
        std::int32_t count = reader.read_int32();
        if (count < 0) return false;
        for (std::int32_t i = 0; i < count; ++i) {
            std::int64_t id = reader.read_int64();
            std::string title = reader.read_string();
            std::int32_t unread = reader.read_int32();
            std::uint32_t unread_u = unread > 0 ? static_cast<std::uint32_t>(unread) : 0U;
            out_chats.append(Chat(ChatId(id), title, unread_u));
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool decodeHistory(const std::vector<std::uint8_t>& data, MessageHistory& out_history) {
    if (data.empty()) return false;
    try {
        TlReader reader(data);
        reader.read_int32(); // constructor
        std::int32_t count = reader.read_int32();
        if (count < 0) return false;
        for (std::int32_t i = 0; i < count; ++i) {
            std::int64_t id = reader.read_int64();
            std::int64_t chat_id = reader.read_int64();
            std::string sender = reader.read_string();
            std::string text = reader.read_string();
            std::int64_t timestamp = reader.read_int64();
            std::int32_t is_outgoing = reader.read_int32();
            out_history.append(Message(MessageId(id), ChatId(chat_id), sender, text, timestamp, is_outgoing != 0));
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool decodeAuth(const std::vector<std::uint8_t>& data, SessionStorage& storage) {
    if (data.empty()) return false;
    try {
        TlReader reader(data);
        std::int32_t constructor = reader.read_int32();
        if (constructor != static_cast<std::int32_t>(0xcd05da1b) && constructor != 1) {
            return false;
        }
        std::vector<std::uint8_t> auth_key = reader.read_bytes();
        std::int64_t server_salt = reader.read_int64();
        std::int64_t session_id = reader.read_int64();
        std::int32_t dc_id = reader.read_int32();
        if (auth_key.empty()) return false;
        SessionData session{std::move(auth_key),
                            static_cast<std::uint64_t>(server_salt),
                            static_cast<std::uint64_t>(session_id),
                            dc_id};
        storage.save(session);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace

TelegramClient::TelegramClient(INetworkTransport& transport, const std::string& session_path)
    : transport_(&transport), storage_(session_path) {}

bool TelegramClient::isAuthorized() const {
    SessionData session;
    return storage_.load(session) && !session.auth_key.empty();
}

bool TelegramClient::requestAuthCode(const PhoneNumber& phone) {
    TlWriter writer;
    writer.write_int32(0x86a);
    writer.write_string(phone.value());
    std::vector<std::uint8_t> response;
    if (!executeRpc(writer.data(), response)) return false;
    return !response.empty();
}

bool TelegramClient::sendCode(const PhoneNumber& phone) {
    return requestAuthCode(phone);
}

bool TelegramClient::signIn(const PhoneNumber& phone, const AuthCode& code) {
    TlWriter writer;
    writer.write_int32(0xbcd);
    writer.write_string(phone.value());
    writer.write_string(code.value());
    std::vector<std::uint8_t> response;
    if (!executeRpc(writer.data(), response)) return false;
    return decodeAuth(response, storage_);
}

bool TelegramClient::checkPassword(const CloudPassword& password) {
    if (password.value().empty()) return false;
    TlWriter writer;
    writer.write_int32(0x301);
    writer.write_string(password.value());
    std::vector<std::uint8_t> response;
    if (!executeRpc(writer.data(), response)) return false;
    return decodeAuth(response, storage_);
}

bool TelegramClient::getDialogs(ChatList& out_chats) {
    TlWriter writer;
    writer.write_int32(0x104);
    std::vector<std::uint8_t> response;
    if (!executeRpc(writer.data(), response)) return false;
    return decodeDialogs(response, out_chats);
}

bool TelegramClient::getHistory(const ChatId& chat_id, MessageHistory& out_history) {
    TlWriter writer;
    writer.write_int32(0x205);
    writer.write_int64(chat_id.value());
    std::vector<std::uint8_t> response;
    if (!executeRpc(writer.data(), response)) return false;
    return decodeHistory(response, out_history);
}

bool TelegramClient::sendMessage(const ChatId& chat_id, const MessageText& text) {
    TlWriter writer;
    writer.write_int32(0x520);
    writer.write_int64(chat_id.value());
    writer.write_string(text.value());
    std::vector<std::uint8_t> response;
    if (!executeRpc(writer.data(), response)) return false;
    return !response.empty();
}

void TelegramClient::logOut() {
    storage_.remove();
}

bool TelegramClient::executeRpc(const std::vector<std::uint8_t>& request,
                               std::vector<std::uint8_t>& response) const {
    if (!transport_) return false;
    return transport_->post("/api", request, response);
}

} // namespace mtproto
