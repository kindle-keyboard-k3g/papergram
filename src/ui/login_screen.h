#ifndef KINDLE_UI_LOGIN_SCREEN_H
#define KINDLE_UI_LOGIN_SCREEN_H

#include "screen.h"
#include "../mtproto/telegram_client.h"

namespace hal {
class AsyncWorker;
}

class ScreenNavigator;

/**
 * @brief Implements phone, verification-code, and two-factor login flows.
 */
class LoginScreen : public IScreen {
public:
    /**
     * @brief Creates a login screen backed by a Telegram client.
     * @param client Client used to send and verify authentication requests.
     * @param navigator Navigator used to continue after successful login.
     * @param worker Optional asynchronous worker for non-blocking requests.
     */
    LoginScreen(mtproto::TelegramClient& client,
                ScreenNavigator& navigator,
                hal::AsyncWorker* worker = nullptr);

    /**
     * @brief Renders the current login step and feedback.
     * @param canvas Destination canvas.
     */
    void render(Canvas& canvas) override;

    /**
     * @brief Handles keyboard input for the current login step.
     * @param event Input event to process.
     */
    void handleInput(const InputEvent& event) override;

    /** @brief Resets the form and enters the phone-number step. */
    void onEnter() override;

    /** @brief Leaves the login screen without additional cleanup. */
    void onExit() override;

    /**
     * @brief Provides login-specific actions for the contextual menu.
     * @return Menu items for resetting the form and clearing tips.
     */
    std::vector<ui::MenuItem> contextualMenuItems() override;

private:
    struct LoginState {
        int step = 0; // 0: Phone, 1: Code, 2: 2FA
        std::string buffer = "+";
        std::string status_message = "Enter your phone number (+1234...)";
        std::string saved_phone = "";
        std::string error_message = "";
        bool is_busy = false;
        ScreenNavigator* navigator = nullptr;
        hal::AsyncWorker* worker = nullptr;
    };

    mtproto::TelegramClient* client_;
    LoginState state_;

    void handleKeyPress(KeyCode code);
    void submitCurrentStep();
    void submitPhoneStep();
    void submitCodeStep();
    void submitPasswordStep();
    void onPhoneSubmitted(bool success);
    void onCodeSubmitted(bool success);
    void onPasswordSubmitted(bool success);
    void eraseLastCharacter();
    void renderToast(Canvas& canvas) const;
    void renderPhoneToast(Canvas& canvas) const;
    void renderCodeToast(Canvas& canvas) const;
    void renderPasswordToast(Canvas& canvas) const;
    void renderErrorToast(Canvas& canvas) const;
    char mapKeyToChar(KeyCode code) const;
};

#endif
