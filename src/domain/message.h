#ifndef KINDLE_DOMAIN_MESSAGE_H
#define KINDLE_DOMAIN_MESSAGE_H

#include "value_objects.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class Message {
public:
    Message(const MessageId& id,
            const ChatId& chat_id,
            const std::string& sender,
            const MessageText& text,
            std::int64_t timestamp,
            bool is_outgoing);
    Message(const MessageId& id,
            const ChatId& chat_id,
            const std::string& sender,
            const std::string& text,
            std::int64_t timestamp,
            bool is_outgoing);

    const MessageId& id() const;
    const ChatId& chatId() const;
    const std::string& sender() const;
    const MessageText& text() const;
    std::int64_t timestamp() const;
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

class MessageHistory {
public:
    void append(const Message& message);
    std::size_t count() const;
    const Message& at(std::size_t index) const;
    std::vector<Message> slice(std::size_t offset, std::size_t amount) const;

private:
    std::vector<Message> messages_;
};

#endif
