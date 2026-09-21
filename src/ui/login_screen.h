#ifndef KINDLE_UI_LOGIN_SCREEN_H
#define KINDLE_UI_LOGIN_SCREEN_H

#include "screen.h"
#include "../mtproto/telegram_client.h"

namespace hal {
class AsyncWorker;
}

class ScreenNavigator;

class LoginScreen : public IScreen {
public:
    LoginScreen(mtproto::TelegramClient& client,
                ScreenNavigator& navigator,
                hal::AsyncWorker* worker = nullptr);

    void render(Canvas& canvas) override;
    void handleInput(const InputEvent& event) override;
    void onEnter() override;
    void onExit() override;
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
