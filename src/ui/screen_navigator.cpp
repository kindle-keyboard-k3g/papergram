#include "screen_navigator.h"

ScreenNavigator::ScreenNavigator() = default;

void ScreenNavigator::setScreens(std::unique_ptr<IScreen> login,
                                std::unique_ptr<IScreen> chat_list,
                                std::unique_ptr<IScreen> conversation) {
    screens_.login = std::move(login);
    screens_.chat_list = std::move(chat_list);
    screens_.conversation = std::move(conversation);
    active_screen_ = screens_.login.get();
}

void ScreenNavigator::showLogin() {
    if (active_screen_) active_screen_->onExit();
    active_screen_ = screens_.login.get();
    if (active_screen_) active_screen_->onEnter();
}

void ScreenNavigator::showChatList() {
    if (active_screen_) active_screen_->onExit();
    active_screen_ = screens_.chat_list.get();
    if (active_screen_) active_screen_->onEnter();
}

void ScreenNavigator::openConversation(const ChatId&, const std::string&) {
    if (active_screen_) active_screen_->onExit();
    active_screen_ = screens_.conversation.get();
    if (active_screen_) active_screen_->onEnter();
}

void ScreenNavigator::render(Canvas& canvas) {
    if (active_screen_) active_screen_->render(canvas);
}

void ScreenNavigator::handleInput(const InputEvent& event) {
    if (active_screen_) active_screen_->handleInput(event);
}

IScreen* ScreenNavigator::currentScreen() const {
    return active_screen_;
}
