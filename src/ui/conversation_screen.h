#ifndef KINDLE_UI_CONVERSATION_SCREEN_H
#define KINDLE_UI_CONVERSATION_SCREEN_H

#include "screen.h"
#include "status_header.h"
#include "../domain/message.h"
#include "../mtproto/telegram_client.h"

class ScreenNavigator;

class ConversationScreen : public IScreen {
public:
    ConversationScreen(const ChatId& chat_id, const std::string& title);
    ConversationScreen(mtproto::TelegramClient& client, ScreenNavigator& navigator);

    void setChat(const ChatId& chat_id, const std::string& title);
    const ChatId& chatId() const;
    void addMessage(const Message& message);
    const std::vector<std::string>& wrappedLines() const;
    const std::string& inputText() const;
    std::string consumeSubmittedMessage();

    void render(Canvas& canvas) override;
    void handleInput(const InputEvent& event) override;
    void onEnter() override;
    void onExit() override;
    std::vector<ui::MenuItem> contextualMenuItems() override;

private:
    struct ConversationState {
        MessageHistory history;
        std::vector<std::string> wrapped_lines;
        std::string input_buffer;
        std::string submitted_buffer;
        ChatId active_chat_id{101};
        std::size_t scroll_offset = 0;
        ScreenNavigator* navigator = nullptr;
        mtproto::TelegramClient* client = nullptr;
    };

    StatusHeader header_;
    ConversationState state_;

    void renderBubble(Canvas& canvas, const Message& msg, int& y) const;
    void renderInputBar(Canvas& canvas) const;
    void sendMessage();
    char mapKeyToChar(KeyCode code) const;
    void wrapText(const std::string& text);
};

#endif
