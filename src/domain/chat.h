#ifndef KINDLE_DOMAIN_CHAT_H
#define KINDLE_DOMAIN_CHAT_H

#include "value_objects.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class ChatTitle {
public:
    explicit ChatTitle(const std::string& value);

    const std::string& value() const;

private:
    std::string value_;
};

using Title = ChatTitle;

class Chat {
public:
    Chat(const ChatId& id, const std::string& title, std::uint32_t unread_count = 0U);
    Chat(const ChatId& id, const ChatTitle& title, std::uint32_t unread_count = 0U);

    const ChatId& id() const;
    const ChatTitle& title() const;
    std::uint32_t unreadCount() const;
    void markRead();
    void incrementUnread();

private:
    struct Details {
        ChatId id;
        ChatTitle title;
    };

    Details details_;
    std::uint32_t unread_count_;
};

class ChatList {
public:
    void append(const Chat& chat);
    std::size_t count() const;
    Chat& selectNext();
    Chat& selectPrevious();
    Chat& selectedChat();
    const Chat& selectedChat() const;
    Chat& at(std::size_t index);
    const Chat& at(std::size_t index) const;

private:
    void ensureNotEmpty() const;

    std::vector<Chat> chats_;
    std::size_t selected_index_ = 0U;
};

#endif
