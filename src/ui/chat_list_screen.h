#ifndef KINDLE_UI_CHAT_LIST_SCREEN_H
#define KINDLE_UI_CHAT_LIST_SCREEN_H

#include "screen.h"
#include "status_header.h"
#include "../domain/chat.h"
#include "../mtproto/telegram_client.h"

namespace hal {
class AsyncWorker;
}

class ScreenNavigator;

class ChatListScreen : public IScreen {
public:
    enum class LoadStatus {
        LOADING,
        READY,
        EMPTY,
        ERROR
    };

    ChatListScreen(mtproto::TelegramClient& client,
                   ScreenNavigator& navigator,
                   hal::AsyncWorker* worker = nullptr);

    void render(Canvas& canvas) override;
    void handleInput(const InputEvent& event) override;
    void onEnter() override;
    void onExit() override;
    std::vector<ui::MenuItem> contextualMenuItems() override;

private:
    struct ScreenState {
        ChatList chats;
        ScreenNavigator* navigator = nullptr;
        mtproto::TelegramClient* client = nullptr;
        hal::AsyncWorker* worker = nullptr;
        LoadStatus status = LoadStatus::LOADING;
    };

    StatusHeader header_;
    ScreenState state_;

    void fetchDialogs();
    void onDialogsLoaded(bool success, ChatList fetched_chats);
    void renderChatRow(Canvas& canvas, std::size_t index, int y) const;
    void renderStateMessage(Canvas& canvas) const;
};

#endif
