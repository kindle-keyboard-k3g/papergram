#ifndef KINDLE_UI_SCREEN_NAVIGATOR_H
#define KINDLE_UI_SCREEN_NAVIGATOR_H

#include "screen.h"
#include "kindle_menu.h"
#include "../domain/value_objects.h"
#include <functional>
#include <memory>
#include <optional>
#include <vector>

class ScreenNavigator {
public:
    ScreenNavigator();

    void setScreens(std::unique_ptr<IScreen> login,
                    std::unique_ptr<IScreen> chat_list,
                    std::unique_ptr<IScreen> conversation);
    void setScreensaver(std::unique_ptr<IScreen> screensaver);

    void showLogin();
    void showChatList();
    void openConversation(const ChatId& chat_id, const std::string& chat_title);
    void lockScreen();
    void unlockScreen();
    bool isLocked() const;

    std::optional<ChatId> activeConversationId() const;
    bool isMenuOpen() const;
    void openMenu();
    void closeMenu();
    void setRefreshCallback(std::function<void()> on_refresh);
    void setExitCallback(std::function<void()> on_exit);

    void render(Canvas& canvas);
    void handleInput(const InputEvent& event);
    IScreen* currentScreen() const;

private:
    struct NavigationState {
        std::unique_ptr<IScreen> login;
        std::unique_ptr<IScreen> chat_list;
        std::unique_ptr<IScreen> conversation;
        std::unique_ptr<IScreen> screensaver;
        IScreen* active_screen = nullptr;
        IScreen* previous_active_screen = nullptr;
        std::optional<ChatId> active_conversation_id{std::nullopt};
        bool locked = false;
        std::function<void()> refresh_cb;
        std::function<void()> exit_cb;
    };

    NavigationState state_;
    ui::KindleMenu menu_;

    void populateMenu();
};

#endif
