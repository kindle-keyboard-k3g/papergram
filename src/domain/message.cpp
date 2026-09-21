#include "message.h"

#include <stdexcept>

namespace {

void requireSender(const std::string& sender) {
    if (sender.empty() || sender.size() > 255U) {
        throw std::invalid_argument("Message sender length is out of range");
    }
}

}  // namespace

Message::Message(const MessageId& id,
                 const ChatId& chat_id,
                 const std::string& sender,
                 const MessageText& text,
                 std::int64_t timestamp,
                 bool is_outgoing)
    : data_{id, chat_id, sender, text, timestamp, is_outgoing} {
    requireSender(data_.sender);
}

Message::Message(const MessageId& id,
                 const ChatId& chat_id,
                 const std::string& sender,
                 const std::string& text,
                 std::int64_t timestamp,
                 bool is_outgoing)
    : Message(id, chat_id, sender, MessageText(text), timestamp, is_outgoing) {}

const MessageId& Message::id() const {
    return data_.id;
}

const ChatId& Message::chatId() const {
    return data_.chat_id;
}

const std::string& Message::sender() const {
    return data_.sender;
}

const MessageText& Message::text() const {
    return data_.text;
}

std::int64_t Message::timestamp() const {
    return data_.timestamp;
}

bool Message::isOutgoing() const {
    return data_.is_outgoing;
}

void MessageHistory::append(const Message& message) {
    messages_.push_back(message);
}

std::size_t MessageHistory::count() const {
    return messages_.size();
}

const Message& MessageHistory::at(std::size_t index) const {
    return messages_.at(index);
}

std::vector<Message> MessageHistory::slice(std::size_t offset, std::size_t amount) const {
    if (offset >= messages_.size() || amount == 0U) {
        return {};
    }
    std::size_t end = messages_.size();
    if (amount <= messages_.size() - offset) {
        end = offset + amount;
    }
    return std::vector<Message>(messages_.begin() + static_cast<std::ptrdiff_t>(offset),
                                messages_.begin() + static_cast<std::ptrdiff_t>(end));
}
