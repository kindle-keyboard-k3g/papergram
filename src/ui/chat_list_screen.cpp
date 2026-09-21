#include "chat_list_screen.h"
#include "screen_navigator.h"

ChatListScreen::ChatListScreen(mtproto::TelegramClient& client, ScreenNavigator& navigator)
    : header_("Kindle Telegram - Chats") {
    state_.client = &client;
    state_.navigator = &navigator;
}

void ChatListScreen::onEnter() {
    if (state_.client) state_.client->getDialogs(state_.chats);
}

void ChatListScreen::onExit() {}

void ChatListScreen::render(Canvas& canvas) {
    canvas.clear(GrayscaleColor::WHITE);
    header_.render(canvas);
    int y = 40;
    for (std::size_t i = 0; i < state_.chats.count() && i < 10; ++i) {
        renderChatRow(canvas, i, y);
        y += 50;
    }
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
    if (!event.pressed) return;
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
        if (state_.client) state_.client->getDialogs(state_.chats);
    });
    items.emplace_back(ui::MenuLabel("Mark All Read"), [this]() {
        for (std::size_t i = 0; i < state_.chats.count(); ++i) {
            state_.chats.at(i).markRead();
        }
    });
    return items;
}

