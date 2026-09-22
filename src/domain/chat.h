#ifndef KINDLE_DOMAIN_CHAT_H
#define KINDLE_DOMAIN_CHAT_H

#include "value_objects.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Validated display title for a chat.
 */
class ChatTitle {
public:
    /**
     * @brief Creates a chat title.
     * @param value Non-empty title of at most 255 bytes.
     * @throws std::invalid_argument If the title is empty or exceeds 255 bytes.
     */
    explicit ChatTitle(const std::string& value);

    /**
     * @brief Returns the chat title text.
     * @return The stored title.
     */
    const std::string& value() const;

private:
    std::string value_;
};

/**
 * @brief Backward-compatible alias for ChatTitle.
 */
using Title = ChatTitle;

/**
 * @brief Domain entity representing a Telegram chat and its unread count.
 */
class Chat {
public:
    /**
     * @brief Creates a chat from a title string.
     * @param id Chat identifier.
     * @param title Non-empty chat title of at most 255 bytes.
     * @param unread_count Initial unread message count.
     * @throws std::invalid_argument If title violates the ChatTitle constraints.
     */
    Chat(const ChatId& id, const std::string& title, std::uint32_t unread_count = 0U);

    /**
     * @brief Creates a chat from a validated title.
     * @param id Chat identifier.
     * @param title Validated chat title.
     * @param unread_count Initial unread message count.
     */
    Chat(const ChatId& id, const ChatTitle& title, std::uint32_t unread_count = 0U);

    /**
     * @brief Returns the chat identifier.
     * @return The validated chat ID.
     */
    const ChatId& id() const;

    /**
     * @brief Returns the chat title.
     * @return The validated chat title.
     */
    const ChatTitle& title() const;

    /**
     * @brief Returns the current unread message count.
     * @return Number of unread messages.
     */
    std::uint32_t unreadCount() const;

    /**
     * @brief Marks every message in the chat as read.
     */
    void markRead();

    /**
     * @brief Increments the unread message count by one.
     */
    void incrementUnread();

private:
    struct Details {
        ChatId id;
        ChatTitle title;
    };

    Details details_;
    std::uint32_t unread_count_;
};

/**
 * @brief Ordered chat collection with wrap-around selection.
 */
class ChatList {
public:
    /**
     * @brief Appends a chat to the end of the list.
     * @param chat Chat to append.
     */
    void append(const Chat& chat);

    /**
     * @brief Returns the number of chats in the list.
     * @return Current chat count.
     */
    std::size_t count() const;

    /**
     * @brief Selects and returns the next chat, wrapping at the end.
     * @return Mutable reference to the newly selected chat.
     * @throws std::out_of_range If the list is empty.
     */
    Chat& selectNext();

    /**
     * @brief Selects and returns the previous chat, wrapping at the beginning.
     * @return Mutable reference to the newly selected chat.
     * @throws std::out_of_range If the list is empty.
     */
    Chat& selectPrevious();

    /**
     * @brief Returns the currently selected chat.
     * @return Mutable reference to the selected chat.
     * @throws std::out_of_range If the list is empty.
     */
    Chat& selectedChat();

    /**
     * @brief Returns the currently selected chat without allowing mutation.
     * @return Const reference to the selected chat.
     * @throws std::out_of_range If the list is empty.
     */
    const Chat& selectedChat() const;

    /**
     * @brief Returns a chat by zero-based index.
     * @param index Index of the requested chat.
     * @return Mutable reference to the chat at index.
     * @throws std::out_of_range If index is not in the list.
     */
    Chat& at(std::size_t index);

    /**
     * @brief Returns a chat by zero-based index without allowing mutation.
     * @param index Index of the requested chat.
     * @return Const reference to the chat at index.
     * @throws std::out_of_range If index is not in the list.
     */
    const Chat& at(std::size_t index) const;

private:
    void ensureNotEmpty() const;

    std::vector<Chat> chats_;
    std::size_t selected_index_ = 0U;
};

#endif
