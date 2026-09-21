#include "login_screen.h"
#include "screen_navigator.h"

LoginScreen::LoginScreen(mtproto::TelegramClient& client, ScreenNavigator& navigator)
    : client_(&client) {
    state_.navigator = &navigator;
}

void LoginScreen::onEnter() {
    state_.step = 0;
    state_.buffer = "+";
    state_.status_message = "Enter your phone number (+1234...)";
}

void LoginScreen::onExit() {}

void LoginScreen::render(Canvas& canvas) {
    canvas.clear(GrayscaleColor::WHITE);
    canvas.blitText(ScreenCoordinate(40, 60), "Kindle Telegram Client", GrayscaleColor::BLACK);
    canvas.drawLine(ScreenCoordinate(40, 84), ScreenCoordinate(560, 84), GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(40, 120), state_.status_message, GrayscaleColor::DARK_GRAY);
    canvas.drawRect(BoundingBox(40, 150, 560, 190), GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(50, 162), state_.buffer + "_", GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(40, 220), "[Enter] Continue   [Del] Erase", GrayscaleColor::DARK_GRAY);
}

void LoginScreen::handleInput(const InputEvent& event) {
    if (!event.pressed) return;
    if (event.code == KeyCode::KEY_ENTER) {
        submitCurrentStep();
        return;
    }
    if (event.code == KeyCode::KEY_BACKSPACE) {
        if (!state_.buffer.empty()) state_.buffer.pop_back();
        return;
    }
    handleKeyPress(event.code);
}

void LoginScreen::handleKeyPress(KeyCode code) {
    char ch = mapKeyToChar(code);
    if (ch != '\0') state_.buffer.push_back(ch);
}

void LoginScreen::submitCurrentStep() {
    if (state_.step == 0) {
        state_.step = 1;
        state_.buffer = "";
        state_.status_message = "Enter 5-digit verification code:";
        return;
    }
    if (state_.step == 1) {
        state_.navigator->showChatList();
    }
}

char LoginScreen::mapKeyToChar(KeyCode code) const {
    if (code >= KeyCode::KEY_0 && code <= KeyCode::KEY_9) {
        return '0' + (static_cast<int>(code) - static_cast<int>(KeyCode::KEY_0));
    }
    if (code >= KeyCode::KEY_A && code <= KeyCode::KEY_Z) {
        return 'a' + (static_cast<int>(code) - static_cast<int>(KeyCode::KEY_A));
    }
    if (code == KeyCode::KEY_SPACE) return ' ';
    return '\0';
}
