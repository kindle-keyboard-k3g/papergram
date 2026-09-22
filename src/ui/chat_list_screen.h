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

/**
 * @brief Displays Telegram dialogs and handles chat selection.
 */
class ChatListScreen : public IScreen {
public:
    /** @brief Describes the state of the dialog-list request. */
    enum class LoadStatus {
        /** @brief Dialogs are being fetched. */
        LOADING,
        /** @brief Dialogs were loaded and can be displayed. */
        READY,
        /** @brief The account has no dialogs to display. */
        EMPTY,
        /** @brief Dialog loading failed. */
        ERROR
    };

    /**
     * @brief Creates a chat-list screen backed by a Telegram client.
     * @param client Client used to fetch dialogs.
     * @param navigator Navigator used to open a selected conversation.
     * @param worker Optional asynchronous worker for non-blocking requests.
     */
    ChatListScreen(mtproto::TelegramClient& client,
                   ScreenNavigator& navigator,
                   hal::AsyncWorker* worker = nullptr);

    /**
     * @brief Renders the header, dialogs, or current loading state.
     * @param canvas Destination canvas.
     */
    void render(Canvas& canvas) override;

    /**
     * @brief Handles navigation and conversation-opening input.
     * @param event Input event to process.
     */
    void handleInput(const InputEvent& event) override;

    /** @brief Starts loading dialogs when the screen becomes active. */
    void onEnter() override;

    /** @brief Leaves the chat-list screen without additional cleanup. */
    void onExit() override;

    /**
     * @brief Provides chat-list actions for the contextual menu.
     * @return Menu items for refreshing chats and marking them read.
     */
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
