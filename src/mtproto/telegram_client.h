#ifndef KINDLE_MTPROTO_TELEGRAM_CLIENT_H
#define KINDLE_MTPROTO_TELEGRAM_CLIENT_H

#include "../domain/chat.h"
#include "../domain/message.h"
#include "../domain/value_objects.h"
#include "../hal/network_transport.h"
#include "session_storage.h"

#include <string>

namespace mtproto {

class TelegramClient {
public:
    TelegramClient(INetworkTransport& transport,
                   const std::string& session_path = "/mnt/us/telegram/session.dat");

    bool isAuthorized() const;
    bool requestAuthCode(const PhoneNumber& phone);
    bool sendCode(const PhoneNumber& phone);
    bool signIn(const PhoneNumber& phone, const AuthCode& code);
    bool checkPassword(const CloudPassword& password);
    bool getDialogs(ChatList& out_chats);
    bool getHistory(const ChatId& chat_id, MessageHistory& out_history);
    bool sendMessage(const ChatId& chat_id, const MessageText& text);
    void logOut();

private:
    INetworkTransport* transport_;
    SessionStorage storage_;

    bool executeRpc(const std::vector<std::uint8_t>& request,
                    std::vector<std::uint8_t>& response) const;
};

}

#endif
