#ifndef KINDLE_UI_CHAT_LIST_SCREEN_H
#define KINDLE_UI_CHAT_LIST_SCREEN_H

#include "screen.h"
#include "status_header.h"
#include "../domain/chat.h"
#include "../mtproto/telegram_client.h"

class ScreenNavigator;

class ChatListScreen : public IScreen {
public:
    ChatListScreen(mtproto::TelegramClient& client, ScreenNavigator& navigator);

    void render(Canvas& canvas) override;
    void handleInput(const InputEvent& event) override;
    void onEnter() override;
    void onExit() override;

private:
    struct ScreenState {
        ChatList chats;
        ScreenNavigator* navigator = nullptr;
        mtproto::TelegramClient* client = nullptr;
    };

    StatusHeader header_;
    ScreenState state_;

    void renderChatRow(Canvas& canvas, std::size_t index, int y) const;
};

#endif
