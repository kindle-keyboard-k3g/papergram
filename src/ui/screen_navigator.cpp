#include "screen_navigator.h"
#include "conversation_screen.h"

namespace {
void updateConversationTarget(IScreen* screen, const ChatId& chat_id, const std::string& title) {
    auto* conversation = dynamic_cast<ConversationScreen*>(screen);
    if (!conversation) return;
    conversation->setChat(chat_id, title);
}
}

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
    state_.active_conversation_id = std::nullopt;
    if (state_.active_screen) state_.active_screen->onExit();
    state_.active_screen = state_.login.get();
    if (state_.active_screen) state_.active_screen->onEnter();
}

void ScreenNavigator::showChatList() {
    if (state_.locked) return;
    state_.active_conversation_id = std::nullopt;
    if (state_.active_screen) state_.active_screen->onExit();
    state_.active_screen = state_.chat_list.get();
    if (state_.active_screen) state_.active_screen->onEnter();
}

void ScreenNavigator::openConversation(const ChatId& chat_id, const std::string& chat_title) {
    if (state_.locked) return;
    state_.active_conversation_id = chat_id;
    updateConversationTarget(state_.conversation.get(), chat_id, chat_title);
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

std::optional<ChatId> ScreenNavigator::activeConversationId() const {
    return state_.active_conversation_id;
}

bool ScreenNavigator::isMenuOpen() const {
    return menu_.isOpen();
}

void ScreenNavigator::openMenu() {
    if (state_.locked) return;
    populateMenu();
}

void ScreenNavigator::closeMenu() {
    menu_.close();
}

void ScreenNavigator::setRefreshCallback(std::function<void()> on_refresh) {
    state_.refresh_cb = std::move(on_refresh);
}

void ScreenNavigator::setExitCallback(std::function<void()> on_exit) {
    state_.exit_cb = std::move(on_exit);
}

void ScreenNavigator::populateMenu() {
    ui::MenuList list;
    if (state_.active_screen) {
        auto contextual = state_.active_screen->contextualMenuItems();
        for (const auto& item : contextual) {
            list.append(item);
        }
    }
    list.append(ui::MenuItem(ui::MenuLabel("Shortcuts & Help"), []() {}, true));
    list.append(ui::MenuItem(ui::MenuLabel("Refresh Screen"), [this]() {
        if (state_.refresh_cb) state_.refresh_cb();
    }));
    list.append(ui::MenuItem(ui::MenuLabel("Log Out"), [this]() {
        showLogin();
    }));
    list.append(ui::MenuItem(ui::MenuLabel("Exit Papergram"), [this]() {
        if (state_.exit_cb) state_.exit_cb();
    }));

    menu_.open(list);
}

void ScreenNavigator::render(Canvas& canvas) {
    if (state_.active_screen) state_.active_screen->render(canvas);
    if (menu_.isOpen()) menu_.render(canvas);
}

void ScreenNavigator::handleInput(const InputEvent& event) {
    if (event.pressed && event.code == KeyCode::KEY_MENU) {
        if (menu_.isOpen()) {
            menu_.close();
            return;
        }
        openMenu();
        return;
    }
    if (menu_.isOpen()) {
        menu_.handleInput(event);
        return;
    }
    if (state_.active_screen) state_.active_screen->handleInput(event);
}

IScreen* ScreenNavigator::currentScreen() const {
    return state_.active_screen;
}
