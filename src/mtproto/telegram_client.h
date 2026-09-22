#ifndef KINDLE_MTPROTO_TELEGRAM_CLIENT_H
#define KINDLE_MTPROTO_TELEGRAM_CLIENT_H

#include "../domain/chat.h"
#include "../domain/message.h"
#include "../domain/value_objects.h"
#include "../hal/network_transport.h"
#include "session_storage.h"

#include <string>

namespace mtproto {

/**
 * @brief Provides high-level Telegram authentication and messaging operations.
 *
 * The client uses the supplied transport for RPC requests and persists
 * authorization state through a SessionStorage instance.
 */
class TelegramClient {
public:
    /**
     * @brief Creates a Telegram client using a network transport.
     *
     * @param transport Transport used to post Telegram RPC requests.
     * @param session_path File path used to persist authorization state.
     */
    TelegramClient(INetworkTransport& transport,
                   const std::string& session_path = "/mnt/us/telegram/session.dat");

    /**
     * @brief Checks whether a non-empty authorization key is persisted.
     *
     * @return `true` when valid authorized session data is available.
     */
    bool isAuthorized() const;

    /**
     * @brief Checks whether sign-in requires a cloud password.
     *
     * @return `true` after the server reports that two-factor authentication is
     *         required and before the password is accepted.
     */
    bool isPasswordNeeded() const;

    /**
     * @brief Requests an authentication code for a phone number.
     *
     * @param phone Phone number for the Telegram account.
     * @return `true` when the request is sent and a response is received.
     */
    bool requestAuthCode(const PhoneNumber& phone);

    /**
     * @brief Sends an authentication-code request for a phone number.
     *
     * This is the public convenience name for requestAuthCode().
     *
     * @param phone Phone number for the Telegram account.
     * @return `true` when the request is sent and a response is received.
     */
    bool sendCode(const PhoneNumber& phone);

    /**
     * @brief Signs in with a phone number and authentication code.
     *
     * If Telegram requires two-factor authentication, this method returns
     * `false` and isPasswordNeeded() becomes `true`.
     *
     * @param phone Phone number receiving the authentication code.
     * @param code Authentication code supplied by Telegram.
     * @return `true` when authorization succeeds and is persisted.
     */
    bool signIn(const PhoneNumber& phone, const AuthCode& code);

    /**
     * @brief Completes sign-in with a Telegram cloud password.
     *
     * @param password Cloud password required by the account.
     * @return `true` when authorization succeeds and is persisted.
     */
    bool checkPassword(const CloudPassword& password);

    /**
     * @brief Retrieves the account's dialogs.
     *
     * @param out_chats Collection to which decoded chats are appended.
     * @return `true` when the request and response decoding succeed.
     */
    bool getDialogs(ChatList& out_chats);

    /**
     * @brief Retrieves message history for a chat.
     *
     * @param chat_id Identifier of the chat to retrieve.
     * @param out_history Collection to which decoded messages are appended.
     * @return `true` when the request and response decoding succeed.
     */
    bool getHistory(const ChatId& chat_id, MessageHistory& out_history);

    /**
     * @brief Sends a text message to a chat.
     *
     * @param chat_id Identifier of the destination chat.
     * @param text Message text to send.
     * @return `true` when the request is sent and a non-empty response is received.
     */
    bool sendMessage(const ChatId& chat_id, const MessageText& text);

    /**
     * @brief Deletes persisted authorization state and clears password state.
     */
    void logOut();

private:
    struct ClientState {
        SessionStorage storage;
        bool password_needed = false;
        explicit ClientState(const std::string& path) : storage(path) {}
    };

    INetworkTransport* transport_;
    ClientState state_;

    bool executeRpc(const std::vector<std::uint8_t>& request,
                    std::vector<std::uint8_t>& response) const;
};

}

#endif
