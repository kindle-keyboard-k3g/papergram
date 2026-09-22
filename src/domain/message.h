#ifndef KINDLE_DOMAIN_MESSAGE_H
#define KINDLE_DOMAIN_MESSAGE_H

#include "value_objects.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Immutable domain entity representing a Telegram message.
 */
class Message {
public:
    /**
     * @brief Creates a message from validated value objects.
     * @param id Message identifier.
     * @param chat_id Identifier of the chat containing the message.
     * @param sender Display name or identifier of the sender.
     * @param text Validated message body.
     * @param timestamp Message timestamp, represented as Unix time.
     * @param is_outgoing Whether the message was sent by the local user.
     * @throws std::invalid_argument If sender is empty or exceeds 255 bytes.
     */
    Message(const MessageId& id,
            const ChatId& chat_id,
            const std::string& sender,
            const MessageText& text,
            std::int64_t timestamp,
            bool is_outgoing);

    /**
     * @brief Creates a message and validates its text value.
     * @param id Message identifier.
     * @param chat_id Identifier of the chat containing the message.
     * @param sender Display name or identifier of the sender.
     * @param text Non-empty valid UTF-8 message body of at most 4096 bytes.
     * @param timestamp Message timestamp, represented as Unix time.
     * @param is_outgoing Whether the message was sent by the local user.
     * @throws std::invalid_argument If sender or text violates its domain rules.
     */
    Message(const MessageId& id,
            const ChatId& chat_id,
            const std::string& sender,
            const std::string& text,
            std::int64_t timestamp,
            bool is_outgoing);

    /**
     * @brief Returns the message identifier.
     * @return The validated message ID.
     */
    const MessageId& id() const;

    /**
     * @brief Returns the containing chat identifier.
     * @return The validated chat ID.
     */
    const ChatId& chatId() const;

    /**
     * @brief Returns the sender label.
     * @return The sender text.
     */
    const std::string& sender() const;

    /**
     * @brief Returns the validated message body.
     * @return The message text value object.
     */
    const MessageText& text() const;

    /**
     * @brief Returns the message timestamp.
     * @return Unix timestamp associated with the message.
     */
    std::int64_t timestamp() const;

    /**
     * @brief Indicates whether the message was sent by the local user.
     * @return true for an outgoing message; otherwise false.
     */
    bool isOutgoing() const;

private:
    struct Data {
        MessageId id;
        ChatId chat_id;
        std::string sender;
        MessageText text;
        std::int64_t timestamp;
        bool is_outgoing;
    };

    Data data_;
};

/**
 * @brief Ordered collection of messages with bounded history access.
 */
class MessageHistory {
public:
    /**
     * @brief Appends a message to the end of the history.
     * @param message Message to append.
     */
    void append(const Message& message);

    /**
     * @brief Returns the number of messages in the history.
     * @return Current message count.
     */
    std::size_t count() const;

    /**
     * @brief Returns a message by zero-based index.
     * @param index Index of the requested message.
     * @return Const reference to the message at index.
     * @throws std::out_of_range If index is not in the history.
     */
    const Message& at(std::size_t index) const;

    /**
     * @brief Copies a contiguous range of messages.
     * @param offset Zero-based index of the first message to include.
     * @param amount Maximum number of messages to include.
     * @return A copy of the requested range, truncated at the end of the history;
     *         empty when offset is out of range or amount is zero.
     */
    std::vector<Message> slice(std::size_t offset, std::size_t amount) const;

private:
    std::vector<Message> messages_;
};

#endif
