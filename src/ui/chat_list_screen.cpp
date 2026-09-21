#include "chat_list_screen.h"
#include "screen_navigator.h"
#include "../hal/async_worker.h"

ChatListScreen::ChatListScreen(mtproto::TelegramClient& client,
                               ScreenNavigator& navigator,
                               hal::AsyncWorker* worker)
    : header_("Kindle Telegram - Chats") {
    state_.client = &client;
    state_.navigator = &navigator;
    state_.worker = worker;
}

void ChatListScreen::onEnter() {
    fetchDialogs();
}

void ChatListScreen::onExit() {}

void ChatListScreen::fetchDialogs() {
    state_.status = LoadStatus::LOADING;
    state_.chats = ChatList();
    if (!state_.worker) {
        ChatList temp;
        bool ok = state_.client && state_.client->getDialogs(temp);
        onDialogsLoaded(ok, std::move(temp));
        return;
    }
    state_.worker->postTask(
        [this]() {
            ChatList temp;
            bool ok = state_.client && state_.client->getDialogs(temp);
            onDialogsLoaded(ok, std::move(temp));
        },
        nullptr
    );
}

void ChatListScreen::onDialogsLoaded(bool success, ChatList fetched_chats) {
    if (!success) {
        state_.status = LoadStatus::ERROR;
        return;
    }
    if (fetched_chats.count() == 0) {
        state_.status = LoadStatus::EMPTY;
        return;
    }
    state_.chats = std::move(fetched_chats);
    state_.status = LoadStatus::READY;
}

void ChatListScreen::render(Canvas& canvas) {
    canvas.clear(GrayscaleColor::WHITE);
    header_.render(canvas);
    if (state_.status != LoadStatus::READY) {
        renderStateMessage(canvas);
        return;
    }
    int y = 40;
    for (std::size_t i = 0; i < state_.chats.count() && i < 10; ++i) {
        renderChatRow(canvas, i, y);
        y += 50;
    }
}

void ChatListScreen::renderStateMessage(Canvas& canvas) const {
    if (state_.status == LoadStatus::LOADING) {
        canvas.blitText(ScreenCoordinate(40, 100), "Loading conversations...", GrayscaleColor::DARK_GRAY);
        return;
    }
    if (state_.status == LoadStatus::EMPTY) {
        canvas.blitText(ScreenCoordinate(40, 100), "No conversations found.", GrayscaleColor::DARK_GRAY);
        return;
    }
    canvas.blitText(ScreenCoordinate(40, 100), "Failed to load chats. [Menu] -> Refresh", GrayscaleColor::BLACK);
}

void ChatListScreen::renderChatRow(Canvas& canvas, std::size_t index, int y) const {
    const Chat& chat = state_.chats.at(index);
    bool selected = (&chat == &state_.chats.selectedChat());
    if (selected) canvas.fillRect(BoundingBox(10, y, 590, y + 42), GrayscaleColor::LIGHT_GRAY);
    canvas.blitText(ScreenCoordinate(20, y + 8), chat.title().value(), GrayscaleColor::BLACK);
    if (chat.unreadCount() > 0) {
        std::string badge = "[" + std::to_string(chat.unreadCount()) + "]";
        canvas.blitText(ScreenCoordinate(520, y + 8), badge, GrayscaleColor::BLACK);
    }
    canvas.drawLine(ScreenCoordinate(10, y + 46), ScreenCoordinate(590, y + 46), GrayscaleColor::DARK_GRAY);
}

void ChatListScreen::handleInput(const InputEvent& event) {
    if (!event.pressed || state_.status != LoadStatus::READY) return;
    if (event.code == KeyCode::KEY_DOWN) state_.chats.selectNext();
    if (event.code == KeyCode::KEY_UP) state_.chats.selectPrevious();
    if (event.code == KeyCode::KEY_PAGEDOWN || event.code == KeyCode::KEY_RIGHT) {
        for (int i = 0; i < 5; ++i) state_.chats.selectNext();
    }
    if (event.code == KeyCode::KEY_PAGEUP || event.code == KeyCode::KEY_LEFT) {
        for (int i = 0; i < 5; ++i) state_.chats.selectPrevious();
    }
    if (event.code == KeyCode::KEY_ENTER && state_.chats.count() > 0) {
        state_.navigator->openConversation(state_.chats.selectedChat().id(),
                                           state_.chats.selectedChat().title().value());
    }
}

std::vector<ui::MenuItem> ChatListScreen::contextualMenuItems() {
    std::vector<ui::MenuItem> items;
    items.emplace_back(ui::MenuLabel("Refresh Chats"), [this]() {
        fetchDialogs();
    });
    items.emplace_back(ui::MenuLabel("Mark All Read"), [this]() {
        for (std::size_t i = 0; i < state_.chats.count(); ++i) {
            state_.chats.at(i).markRead();
        }
    });
    return items;
}
