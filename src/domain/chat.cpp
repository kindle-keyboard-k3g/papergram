#include "chat.h"

#include <stdexcept>

ChatTitle::ChatTitle(const std::string& value) : value_(value) {
    if (value_.empty() || value_.size() > 255U) {
        throw std::invalid_argument("Chat title length is out of range");
    }
}

const std::string& ChatTitle::value() const {
    return value_;
}

Chat::Chat(const ChatId& id, const std::string& title, std::uint32_t unread_count)
    : Chat(id, ChatTitle(title), unread_count) {}

Chat::Chat(const ChatId& id, const ChatTitle& title, std::uint32_t unread_count)
    : details_{id, title}, unread_count_(unread_count) {}

const ChatId& Chat::id() const {
    return details_.id;
}

const ChatTitle& Chat::title() const {
    return details_.title;
}

std::uint32_t Chat::unreadCount() const {
    return unread_count_;
}

void Chat::markRead() {
    unread_count_ = 0U;
}

void Chat::incrementUnread() {
    ++unread_count_;
}

void ChatList::append(const Chat& chat) {
    chats_.push_back(chat);
}

std::size_t ChatList::count() const {
    return chats_.size();
}

Chat& ChatList::selectNext() {
    ensureNotEmpty();
    selected_index_ = (selected_index_ + 1U) % chats_.size();
    return selectedChat();
}

Chat& ChatList::selectPrevious() {
    ensureNotEmpty();
    selected_index_ = selected_index_ == 0U ? chats_.size() - 1U : selected_index_ - 1U;
    return selectedChat();
}

Chat& ChatList::selectedChat() {
    ensureNotEmpty();
    return chats_.at(selected_index_);
}

const Chat& ChatList::selectedChat() const {
    ensureNotEmpty();
    return chats_.at(selected_index_);
}

Chat& ChatList::at(std::size_t index) {
    return chats_.at(index);
}

const Chat& ChatList::at(std::size_t index) const {
    return chats_.at(index);
}

void ChatList::ensureNotEmpty() const {
    if (chats_.empty()) {
        throw std::out_of_range("Cannot select a chat from an empty chat list");
    }
}
