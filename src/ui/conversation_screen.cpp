#include "conversation_screen.h"
#include "screen_navigator.h"

namespace {
std::vector<std::string> splitTextIntoLines(const std::string& text, std::size_t max_len) {
    std::vector<std::string> lines;
    if (text.empty()) {
        lines.push_back("");
        return lines;
    }
    for (std::size_t i = 0; i < text.size(); i += max_len) {
        lines.push_back(text.substr(i, max_len));
    }
    return lines;
}
}

ConversationScreen::ConversationScreen(const ChatId& chat_id, const std::string& title)
    : header_(title) {
    state_.active_chat_id = chat_id;
}

ConversationScreen::ConversationScreen(mtproto::TelegramClient& client, ScreenNavigator& navigator)
    : header_("Telegram Conversation") {
    state_.client = &client;
    state_.navigator = &navigator;
}

void ConversationScreen::setChat(const ChatId& chat_id, const std::string& title) {
    state_.active_chat_id = chat_id;
    header_.setTitle(title);
}

const ChatId& ConversationScreen::chatId() const {
    return state_.active_chat_id;
}

void ConversationScreen::addMessage(const Message& message) {
    state_.history.append(message);
    wrapText(message.text().value());
}

const std::vector<std::string>& ConversationScreen::wrappedLines() const {
    return state_.wrapped_lines;
}

const std::string& ConversationScreen::inputText() const {
    return state_.input_buffer;
}

std::string ConversationScreen::consumeSubmittedMessage() {
    std::string result = state_.submitted_buffer;
    state_.submitted_buffer.clear();
    return result;
}

void ConversationScreen::onEnter() {
    if (state_.client) state_.client->getHistory(state_.active_chat_id, state_.history);
}

void ConversationScreen::onExit() {}

void ConversationScreen::render(Canvas& canvas) {
    canvas.clear(GrayscaleColor::WHITE);
    header_.render(canvas);
    int y = 48;
    for (std::size_t i = state_.scroll_offset; i < state_.history.count() && y < 680; ++i) {
        renderBubble(canvas, state_.history.at(i), y);
    }
    renderInputBar(canvas);
}

void ConversationScreen::renderBubble(Canvas& canvas, const Message& msg, int& y) const {
    int x = msg.isOutgoing() ? 120 : 20;
    std::vector<std::string> lines = splitTextIntoLines(msg.text().value(), 50);
    int bubble_height = 24 + static_cast<int>(lines.size()) * 18;
    canvas.drawRect(BoundingBox(x, y, x + 440, y + bubble_height), GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(x + 10, y + 6), msg.sender(), GrayscaleColor::DARK_GRAY);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        canvas.blitText(ScreenCoordinate(x + 10, y + 22 + static_cast<int>(i) * 18), lines[i], GrayscaleColor::BLACK);
    }
    y += bubble_height + 12;
}

void ConversationScreen::renderInputBar(Canvas& canvas) const {
    canvas.drawLine(ScreenCoordinate(0, 730), ScreenCoordinate(600, 730), GrayscaleColor::BLACK);
    canvas.drawRect(BoundingBox(20, 745, 580, 785), GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(30, 757), state_.input_buffer + "_", GrayscaleColor::BLACK);
}

void ConversationScreen::handleInput(const InputEvent& event) {
    if (!event.pressed) return;
    if (event.code == KeyCode::KEY_BACK && state_.navigator) {
        state_.navigator->showChatList();
        return;
    }
    if (event.code == KeyCode::KEY_ENTER) {
        sendMessage();
        return;
    }
    if (event.code == KeyCode::KEY_BACKSPACE) {
        if (!state_.input_buffer.empty()) state_.input_buffer.pop_back();
        return;
    }
    if (event.code == KeyCode::KEY_PAGEUP) {
        if (state_.scroll_offset > 3) state_.scroll_offset -= 3;
        if (state_.scroll_offset <= 3) state_.scroll_offset = 0;
        return;
    }
    if (event.code == KeyCode::KEY_PAGEDOWN) {
        if (state_.scroll_offset + 3 < state_.history.count()) state_.scroll_offset += 3;
        return;
    }
    char ch = mapKeyToChar(event.code);
    if (ch != '\0') state_.input_buffer.push_back(ch);
}

void ConversationScreen::sendMessage() {
    if (state_.input_buffer.empty()) return;
    state_.submitted_buffer = state_.input_buffer;
    Message msg(MessageId(99), state_.active_chat_id, "You", state_.input_buffer, 1600000100, true);
    addMessage(msg);
    if (state_.client) state_.client->sendMessage(state_.active_chat_id, MessageText(state_.input_buffer));
    state_.input_buffer.clear();
}

char ConversationScreen::mapKeyToChar(KeyCode code) const {
    if (code >= KeyCode::KEY_0 && code <= KeyCode::KEY_9) {
        return '0' + (static_cast<int>(code) - static_cast<int>(KeyCode::KEY_0));
    }
    if (code >= KeyCode::KEY_A && code <= KeyCode::KEY_Z) {
        return 'a' + (static_cast<int>(code) - static_cast<int>(KeyCode::KEY_A));
    }
    if (code == KeyCode::KEY_SPACE) return ' ';
    return '\0';
}

void ConversationScreen::wrapText(const std::string& text) {
    std::vector<std::string> lines = splitTextIntoLines(text, 58);
    state_.wrapped_lines.insert(state_.wrapped_lines.end(), lines.begin(), lines.end());
}
