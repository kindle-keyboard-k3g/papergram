#include "screen_navigator.h"

ScreenNavigator::ScreenNavigator() = default;

void ScreenNavigator::setScreens(std::unique_ptr<IScreen> login,
                                std::unique_ptr<IScreen> chat_list,
                                std::unique_ptr<IScreen> conversation) {
    state_.login = std::move(login);
    state_.chat_list = std::move(chat_list);
    state_.conversation = std::move(conversation);
    state_.active_screen = state_.login.get();
}

void ScreenNavigator::setScreensaver(std::unique_ptr<IScreen> screensaver) {
    state_.screensaver = std::move(screensaver);
}

void ScreenNavigator::showLogin() {
    if (state_.locked) return;
    if (state_.active_screen) state_.active_screen->onExit();
    state_.active_screen = state_.login.get();
    if (state_.active_screen) state_.active_screen->onEnter();
}

void ScreenNavigator::showChatList() {
    if (state_.locked) return;
    if (state_.active_screen) state_.active_screen->onExit();
    state_.active_screen = state_.chat_list.get();
    if (state_.active_screen) state_.active_screen->onEnter();
}

void ScreenNavigator::openConversation(const ChatId&, const std::string&) {
    if (state_.locked) return;
    if (state_.active_screen) state_.active_screen->onExit();
    state_.active_screen = state_.conversation.get();
    if (state_.active_screen) state_.active_screen->onEnter();
}

void ScreenNavigator::lockScreen() {
    if (state_.locked) return;
    if (!state_.screensaver) return;
    state_.previous_active_screen = state_.active_screen;
    if (state_.active_screen) state_.active_screen->onExit();
    state_.active_screen = state_.screensaver.get();
    state_.locked = true;
    if (state_.active_screen) state_.active_screen->onEnter();
}

void ScreenNavigator::unlockScreen() {
    if (!state_.locked) return;
    if (state_.active_screen) state_.active_screen->onExit();
    state_.active_screen = state_.previous_active_screen;
    state_.previous_active_screen = nullptr;
    state_.locked = false;
    if (state_.active_screen) state_.active_screen->onEnter();
}

bool ScreenNavigator::isLocked() const {
    return state_.locked;
}

void ScreenNavigator::render(Canvas& canvas) {
    if (state_.active_screen) state_.active_screen->render(canvas);
}

void ScreenNavigator::handleInput(const InputEvent& event) {
    if (state_.active_screen) state_.active_screen->handleInput(event);
}

IScreen* ScreenNavigator::currentScreen() const {
    return state_.active_screen;
}
