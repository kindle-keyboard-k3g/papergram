#include "test_framework.h"

#include "../src/domain/chat.h"
#include "../src/domain/message.h"
#include "../src/domain/value_objects.h"

#include <stdexcept>
#include <string>

TEST(phone_number_accepts_e164_and_rejects_invalid_values) {
    const PhoneNumber number("+1234567890");
    ASSERT_STR_EQ("+1234567890", number.value());

    bool rejected = false;
    try {
        PhoneNumber invalid("1234567890");
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    ASSERT_TRUE(rejected);
}

TEST(auth_code_requires_five_digits) {
    const AuthCode code("12345");
    ASSERT_STR_EQ("12345", code.value());

    bool rejected = false;
    try {
        AuthCode invalid("1234");
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    ASSERT_TRUE(rejected);
}

TEST(message_text_rejects_empty_and_invalid_utf8) {
    const MessageText text("Olá");
    ASSERT_EQ(4U, text.length());

    bool rejected = false;
    try {
        MessageText invalid(std::string(1, static_cast<char>(0xC3)));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    ASSERT_TRUE(rejected);
}

TEST(bounding_box_validates_order_and_contains_coordinates) {
    const BoundingBox box(10, 20, 100, 200);
    ASSERT_EQ(91, box.width());
    ASSERT_EQ(181, box.height());
    ASSERT_TRUE(box.contains(ScreenCoordinate(10, 20)));
    ASSERT_TRUE(box.contains(ScreenCoordinate(100, 200)));
    ASSERT_FALSE(box.contains(ScreenCoordinate(101, 200)));

    bool rejected = false;
    try {
        BoundingBox invalid(100, 20, 10, 200);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    ASSERT_TRUE(rejected);
}

TEST(chat_list_selects_and_wraps_chats) {
    ChatList chats;
    chats.append(Chat(ChatId(1), "First"));
    chats.append(Chat(ChatId(2), "Second", 3U));

    ASSERT_EQ(2U, chats.count());
    ASSERT_EQ(ChatId(1), chats.selectedChat().id());
    chats.selectNext();
    ASSERT_EQ(ChatId(2), chats.selectedChat().id());
    chats.selectNext();
    ASSERT_EQ(ChatId(1), chats.selectedChat().id());
    chats.selectPrevious();
    ASSERT_EQ(ChatId(2), chats.selectedChat().id());
    chats.selectedChat().markRead();
    ASSERT_EQ(0U, chats.selectedChat().unreadCount());
}

TEST(message_history_slices_messages) {
    const ChatId chat_id(42);
    MessageHistory history;
    history.append(Message(MessageId(1), chat_id, "Alice", "One", 100, false));
    history.append(Message(MessageId(2), chat_id, "Bob", "Two", 200, true));
    history.append(Message(MessageId(3), chat_id, "Alice", "Three", 300, false));

    ASSERT_EQ(3U, history.count());
    ASSERT_STR_EQ("Two", history.at(1).text().value());
    const std::vector<Message> visible = history.slice(1U, 2U);
    ASSERT_EQ(2U, visible.size());
    ASSERT_STR_EQ("Three", visible.at(1).text().value());
}
