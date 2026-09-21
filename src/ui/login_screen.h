#ifndef KINDLE_UI_LOGIN_SCREEN_H
#define KINDLE_UI_LOGIN_SCREEN_H

#include "screen.h"
#include "../mtproto/telegram_client.h"

class ScreenNavigator;

class LoginScreen : public IScreen {
public:
    LoginScreen(mtproto::TelegramClient& client, ScreenNavigator& navigator);

    void render(Canvas& canvas) override;
    void handleInput(const InputEvent& event) override;
    void onEnter() override;
    void onExit() override;

private:
    struct LoginState {
        int step = 0; // 0: Phone, 1: Code, 2: 2FA
        std::string buffer = "+";
        std::string status_message = "Enter your phone number";
        ScreenNavigator* navigator = nullptr;
    };

    mtproto::TelegramClient* client_;
    LoginState state_;

    void handleKeyPress(KeyCode code);
    void submitCurrentStep();
    void eraseLastCharacter();
    void renderToast(Canvas& canvas) const;
    void renderPhoneToast(Canvas& canvas) const;
    void renderCodeToast(Canvas& canvas) const;
    char mapKeyToChar(KeyCode code) const;
};

#endif
