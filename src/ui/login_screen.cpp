#include "login_screen.h"
#include "screen_navigator.h"
#include "toast_notification.h"

#include <vector>

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
    renderToast(canvas);
}

void LoginScreen::handleInput(const InputEvent& event) {
    if (!event.pressed) return;
    if (event.code == KeyCode::KEY_ENTER) {
        submitCurrentStep();
        return;
    }
    if (event.code == KeyCode::KEY_BACKSPACE) {
        eraseLastCharacter();
        return;
    }
    handleKeyPress(event.code);
}

void LoginScreen::eraseLastCharacter() {
    if (state_.step == 0 && state_.buffer.size() <= 1U) {
        state_.buffer = "+";
        return;
    }
    if (state_.buffer.empty()) return;
    state_.buffer.pop_back();
}

void LoginScreen::renderToast(Canvas& canvas) const {
    if (state_.step == 0) {
        renderPhoneToast(canvas);
        return;
    }
    if (state_.step == 1) renderCodeToast(canvas);
}

void LoginScreen::renderPhoneToast(Canvas& canvas) const {
    const std::vector<std::string> lines = {
        "- '+' is already included for country code.",
        "- Hold [ALT] + top row (Q-P) to type 1-0.",
        "- Example: [ALT]+5 [ALT]+5 for Brazil +55"};
    ui::ToastNotification toast(
        BoundingBox(ScreenCoordinate(40, 270), ScreenCoordinate(560, 420)),
        "KEYBOARD TIPS:", lines);
    toast.render(canvas);
}

void LoginScreen::renderCodeToast(Canvas& canvas) const {
    const std::vector<std::string> lines = {
        "- Hold [ALT] + top row (Q-P) for digits.",
        "- Check your Telegram app on phone/PC."};
    ui::ToastNotification toast(
        BoundingBox(ScreenCoordinate(40, 270), ScreenCoordinate(560, 420)),
        "CODE TIPS:", lines);
    toast.render(canvas);
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

std::vector<ui::MenuItem> LoginScreen::contextualMenuItems() {
    std::vector<ui::MenuItem> items;
    items.emplace_back(ui::MenuLabel("Reset Form"), [this]() {
        state_.step = 0;
        state_.buffer = "+";
        state_.status_message = "Enter your phone number";
    });
    items.emplace_back(ui::MenuLabel("Keyboard Tips"), [this]() {
        state_.step = 0;
    });
    return items;
}

