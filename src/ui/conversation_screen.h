#ifndef KINDLE_UI_CONVERSATION_SCREEN_H
#define KINDLE_UI_CONVERSATION_SCREEN_H

#include "screen.h"
#include "status_header.h"
#include "../domain/message.h"
#include "../mtproto/telegram_client.h"

namespace hal {
class AsyncWorker;
}

class ScreenNavigator;

/**
 * @brief Displays a conversation, accepts input, and sends messages.
 */
class ConversationScreen : public IScreen {
public:
    /** @brief Describes the state of the conversation-history request. */
    enum class HistoryStatus {
        /** @brief History is being fetched. */
        LOADING,
        /** @brief History was loaded and can be displayed. */
        READY,
        /** @brief The conversation contains no messages. */
        EMPTY,
        /** @brief History loading failed. */
        ERROR
    };

    /**
     * @brief Creates a standalone conversation with an initial chat target.
     * @param chat_id Identifier of the conversation.
     * @param title Title displayed in the status header.
     */
    ConversationScreen(const ChatId& chat_id, const std::string& title);

    /**
     * @brief Creates a client-backed conversation screen.
     * @param client Client used to fetch history and send messages.
     * @param navigator Navigator used to return to the chat list.
     * @param worker Optional asynchronous worker for non-blocking requests.
     */
    ConversationScreen(mtproto::TelegramClient& client,
                       ScreenNavigator& navigator,
                       hal::AsyncWorker* worker = nullptr);

    /**
     * @brief Changes the active conversation and its displayed title.
     * @param chat_id Identifier of the new conversation.
     * @param title Title displayed in the status header.
     */
    void setChat(const ChatId& chat_id, const std::string& title);

    /**
     * @brief Returns the active conversation identifier.
     * @return Active chat identifier.
     */
    const ChatId& chatId() const;

    /**
     * @brief Appends a message and wraps its text for display.
     * @param message Message to add to the conversation.
     */
    void addMessage(const Message& message);

    /**
     * @brief Returns the accumulated wrapped message lines.
     * @return Read-only wrapped-line collection.
     */
    const std::vector<std::string>& wrappedLines() const;

    /**
     * @brief Returns the text currently entered in the input bar.
     * @return Read-only input text.
     */
    const std::string& inputText() const;

    /**
     * @brief Returns and clears the most recently submitted message.
     * @return Submitted message text, or an empty string when none is pending.
     */
    std::string consumeSubmittedMessage();

    /**
     * @brief Renders the header, history, input bar, and any errors.
     * @param canvas Destination canvas.
     */
    void render(Canvas& canvas) override;

    /**
     * @brief Handles navigation, editing, scrolling, and send input.
     * @param event Input event to process.
     */
    void handleInput(const InputEvent& event) override;

    /** @brief Starts loading history when the screen becomes active. */
    void onEnter() override;

    /** @brief Leaves the conversation screen without additional cleanup. */
    void onExit() override;

    /**
     * @brief Provides conversation actions for the contextual menu.
     * @return Menu items for refreshing history, clearing input, and returning.
     */
    std::vector<ui::MenuItem> contextualMenuItems() override;

private:
    struct ConversationState {
        MessageHistory history;
        std::vector<std::string> wrapped_lines;
        std::string input_buffer;
        std::string submitted_buffer;
        std::string error_message;
        ChatId active_chat_id{101};
        std::size_t scroll_offset = 0;
        ScreenNavigator* navigator = nullptr;
        mtproto::TelegramClient* client = nullptr;
        hal::AsyncWorker* worker = nullptr;
        HistoryStatus status = HistoryStatus::LOADING;
    };

    StatusHeader header_;
    ConversationState state_;

    void fetchHistory();
    void onHistoryLoaded(bool success, MessageHistory fetched_history);
    void onMessageSent(bool success);
    void renderBubble(Canvas& canvas, const Message& msg, int& y) const;
    void renderInputBar(Canvas& canvas) const;
    void renderStateMessage(Canvas& canvas) const;
    void renderErrorToast(Canvas& canvas) const;
    void sendMessage();
    char mapKeyToChar(KeyCode code) const;
    void wrapText(const std::string& text);
};

#endif
