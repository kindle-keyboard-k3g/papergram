#ifndef KINDLE_UI_SCREEN_NAVIGATOR_H
#define KINDLE_UI_SCREEN_NAVIGATOR_H

#include "screen.h"
#include "../domain/value_objects.h"
#include <memory>
#include <vector>

class ScreenNavigator {
public:
    ScreenNavigator();

    void setScreens(std::unique_ptr<IScreen> login,
                    std::unique_ptr<IScreen> chat_list,
                    std::unique_ptr<IScreen> conversation);

    void showLogin();
    void showChatList();
    void openConversation(const ChatId& chat_id, const std::string& chat_title);

    void render(Canvas& canvas);
    void handleInput(const InputEvent& event);
    IScreen* currentScreen() const;

private:
    struct NavigationState {
        std::unique_ptr<IScreen> login;
        std::unique_ptr<IScreen> chat_list;
        std::unique_ptr<IScreen> conversation;
    };

    NavigationState screens_;
    IScreen* active_screen_ = nullptr;
};

#endif
