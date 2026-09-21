#include "test_framework.h"

#include "../src/domain/message.h"
#include "../src/graphics/canvas.h"
#include "../src/ui/conversation_screen.h"

TEST(conversation_screen_wraps_messages_at_fifty_eight_characters) {
    ConversationScreen screen(ChatId(7), "Kindle");
    const std::string long_text(120U, 'x');
    screen.addMessage(Message(MessageId(1), ChatId(7), "Alice", long_text,
                              0, false));

    ASSERT_TRUE(screen.wrappedLines().size() >= 3U);
    for (const std::string& line : screen.wrappedLines()) {
        ASSERT_TRUE(line.size() <= 58U);
    }
}

TEST(conversation_screen_accepts_keyboard_text_and_submits) {
    ConversationScreen screen(ChatId(7), "Kindle");
    screen.handleInput(InputEvent{KeyCode::KEY_H, true});
    screen.handleInput(InputEvent{KeyCode::KEY_I, true});
    ASSERT_STR_EQ("hi", screen.inputText());

    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});
    ASSERT_STR_EQ("hi", screen.consumeSubmittedMessage());
    ASSERT_STR_EQ("", screen.inputText());
}

TEST(conversation_screen_renders_incoming_and_outgoing_messages) {
    ConversationScreen screen(ChatId(7), "Kindle");
    screen.addMessage(Message(MessageId(1), ChatId(7), "Alice", "hello", 0, false));
    screen.addMessage(Message(MessageId(2), ChatId(7), "Me", "sent", 0, true));
    Canvas canvas;

    screen.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(0, 48, 599, 799)));
}
