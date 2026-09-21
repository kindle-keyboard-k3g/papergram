#include "login_screen.h"
#include "screen_navigator.h"
#include "toast_notification.h"
#include "../domain/value_objects.h"
#include "../hal/async_worker.h"

#include <stdexcept>
#include <vector>

LoginScreen::LoginScreen(mtproto::TelegramClient& client,
                         ScreenNavigator& navigator,
                         hal::AsyncWorker* worker)
    : client_(&client) {
    state_.navigator = &navigator;
    state_.worker = worker;
}

void LoginScreen::onEnter() {
    state_.step = 0;
    state_.buffer = "+";
    state_.status_message = "Enter your phone number (+1234...)";
    state_.saved_phone = "";
    state_.error_message = "";
    state_.is_busy = false;
}

void LoginScreen::onExit() {}

void LoginScreen::render(Canvas& canvas) {
    canvas.clear(GrayscaleColor::WHITE);
    canvas.blitText(ScreenCoordinate(40, 60), "Kindle Telegram Client", GrayscaleColor::BLACK);
    canvas.drawLine(ScreenCoordinate(40, 84), ScreenCoordinate(560, 84), GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(40, 120), state_.status_message, GrayscaleColor::DARK_GRAY);
    canvas.drawRect(BoundingBox(40, 150, 560, 190), GrayscaleColor::BLACK);
    std::string display_buffer = state_.step == 2 ? std::string(state_.buffer.size(), '*') : state_.buffer;
    canvas.blitText(ScreenCoordinate(50, 162), display_buffer + "_", GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(40, 220), "[Enter] Continue   [Del] Erase", GrayscaleColor::DARK_GRAY);
    renderToast(canvas);
}

void LoginScreen::handleInput(const InputEvent& event) {
    if (!event.pressed || state_.is_busy) return;
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
    if (!state_.error_message.empty()) {
        renderErrorToast(canvas);
        return;
    }
    if (state_.step == 0) {
        renderPhoneToast(canvas);
        return;
    }
    if (state_.step == 1) {
        renderCodeToast(canvas);
        return;
    }
    renderPasswordToast(canvas);
}

void LoginScreen::renderErrorToast(Canvas& canvas) const {
    const std::vector<std::string> lines = {
        state_.error_message,
        "Press [Del] to edit or try again."
    };
    ui::ToastNotification toast(
        BoundingBox(ScreenCoordinate(40, 270), ScreenCoordinate(560, 420)),
        "ERROR:", lines);
    toast.render(canvas);
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

void LoginScreen::renderPasswordToast(Canvas& canvas) const {
    const std::vector<std::string> lines = {
        "- Enter your Two-Step Verification password.",
        "- Characters are masked for privacy."};
    ui::ToastNotification toast(
        BoundingBox(ScreenCoordinate(40, 270), ScreenCoordinate(560, 420)),
        "2FA PASSWORD TIPS:", lines);
    toast.render(canvas);
}

void LoginScreen::handleKeyPress(KeyCode code) {
    char ch = mapKeyToChar(code);
    if (ch != '\0') state_.buffer.push_back(ch);
}

void LoginScreen::submitCurrentStep() {
    if (state_.step == 0) {
        submitPhoneStep();
        return;
    }
    if (state_.step == 1) {
        submitCodeStep();
        return;
    }
    submitPasswordStep();
}

void LoginScreen::submitPhoneStep() {
    try {
        PhoneNumber phone(state_.buffer);
        state_.error_message = "";
        state_.saved_phone = state_.buffer;
        if (!state_.worker) {
            bool ok = client_ && client_->sendCode(phone);
            onPhoneSubmitted(ok);
            return;
        }
        state_.is_busy = true;
        state_.status_message = "Sending code...";
        state_.worker->postTask(
            [this, phone]() {
                bool ok = client_ && client_->sendCode(phone);
                onPhoneSubmitted(ok);
            },
            nullptr
        );
    } catch (const std::invalid_argument& e) {
        state_.error_message = e.what();
    }
}

void LoginScreen::onPhoneSubmitted(bool success) {
    state_.is_busy = false;
    if (!success) {
        state_.error_message = "Failed to send code. Please try again.";
        return;
    }
    state_.step = 1;
    state_.buffer = "";
    state_.status_message = "Enter 5-digit verification code:";
}

void LoginScreen::submitCodeStep() {
    try {
        AuthCode code(state_.buffer);
        PhoneNumber phone(state_.saved_phone);
        state_.error_message = "";
        if (!state_.worker) {
            bool ok = client_ && client_->signIn(phone, code);
            onCodeSubmitted(ok);
            return;
        }
        state_.is_busy = true;
        state_.status_message = "Signing in...";
        state_.worker->postTask(
            [this, phone, code]() {
                bool ok = client_ && client_->signIn(phone, code);
                onCodeSubmitted(ok);
            },
            nullptr
        );
    } catch (const std::invalid_argument& e) {
        state_.error_message = e.what();
    }
}

void LoginScreen::onCodeSubmitted(bool success) {
    state_.is_busy = false;
    if (success) {
        if (state_.navigator) state_.navigator->showChatList();
        return;
    }
    if (client_ && client_->isPasswordNeeded()) {
        state_.step = 2;
        state_.buffer = "";
        state_.status_message = "Enter 2FA Cloud Password:";
        return;
    }
    state_.error_message = "Invalid code. Please try again.";
}

void LoginScreen::submitPasswordStep() {
    try {
        CloudPassword password(state_.buffer);
        state_.error_message = "";
        if (!state_.worker) {
            bool ok = client_ && client_->checkPassword(password);
            onPasswordSubmitted(ok);
            return;
        }
        state_.is_busy = true;
        state_.status_message = "Verifying password...";
        state_.worker->postTask(
            [this, password]() {
                bool ok = client_ && client_->checkPassword(password);
                onPasswordSubmitted(ok);
            },
            nullptr
        );
    } catch (const std::invalid_argument& e) {
        state_.error_message = e.what();
    }
}

void LoginScreen::onPasswordSubmitted(bool success) {
    state_.is_busy = false;
    if (success) {
        if (state_.navigator) state_.navigator->showChatList();
        return;
    }
    state_.error_message = "Incorrect password. Please try again.";
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
        state_.status_message = "Enter your phone number (+1234...)";
        state_.saved_phone = "";
        state_.error_message = "";
        state_.is_busy = false;
    });
    items.emplace_back(ui::MenuLabel("Keyboard Tips"), [this]() {
        state_.error_message = "";
    });
    return items;
}
