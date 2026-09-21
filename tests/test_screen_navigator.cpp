#include "test_framework.h"
#include "../src/ui/screen_navigator.h"
#include "../src/ui/screensaver_screen.h"
#include "../src/ui/conversation_screen.h"

namespace {
class DummyTestScreen : public IScreen {
public:
    int enter_count = 0;
    int exit_count = 0;

    void render(Canvas&) override {}
    void handleInput(const InputEvent&) override {}
    void onEnter() override { ++enter_count; }
    void onExit() override { ++exit_count; }
};
}

TEST(screen_navigator_lock_and_unlock_restores_previous_screen) {
    ScreenNavigator navigator;
    auto login = std::make_unique<DummyTestScreen>();
    auto chat_list = std::make_unique<DummyTestScreen>();
    auto conv = std::make_unique<DummyTestScreen>();
    auto screensaver = std::make_unique<DummyTestScreen>();

    auto* chat_list_ptr = chat_list.get();
    auto* screensaver_ptr = screensaver.get();

    navigator.setScreens(std::move(login), std::move(chat_list), std::move(conv));
    navigator.setScreensaver(std::move(screensaver));

    navigator.showChatList();
    ASSERT_EQ(chat_list_ptr, navigator.currentScreen());

    navigator.lockScreen();
    ASSERT_TRUE(navigator.isLocked());
    ASSERT_EQ(screensaver_ptr, navigator.currentScreen());

    navigator.unlockScreen();
    ASSERT_FALSE(navigator.isLocked());
    ASSERT_EQ(chat_list_ptr, navigator.currentScreen());
}

TEST(screen_navigator_prevents_navigation_while_locked) {
    ScreenNavigator navigator;
    auto login = std::make_unique<DummyTestScreen>();
    auto chat_list = std::make_unique<DummyTestScreen>();
    auto conv = std::make_unique<DummyTestScreen>();
    auto screensaver = std::make_unique<DummyTestScreen>();

    auto* screensaver_ptr = screensaver.get();

    navigator.setScreens(std::move(login), std::move(chat_list), std::move(conv));
    navigator.setScreensaver(std::move(screensaver));

    navigator.showChatList();
    navigator.lockScreen();
    ASSERT_TRUE(navigator.isLocked());

    navigator.showLogin();
    ASSERT_EQ(screensaver_ptr, navigator.currentScreen());

    navigator.openConversation(ChatId(123), "Test");
    ASSERT_EQ(screensaver_ptr, navigator.currentScreen());
}

TEST(screen_navigator_lock_without_screensaver_is_noop) {
    ScreenNavigator navigator;
    auto login = std::make_unique<DummyTestScreen>();
    auto* login_ptr = login.get();
    navigator.setScreens(std::move(login), nullptr, nullptr);

    navigator.lockScreen();
    ASSERT_FALSE(navigator.isLocked());
    ASSERT_EQ(login_ptr, navigator.currentScreen());
}

TEST(screen_navigator_redundant_lock_and_unlock) {
    ScreenNavigator navigator;
    auto login = std::make_unique<DummyTestScreen>();
    auto chat_list = std::make_unique<DummyTestScreen>();
    auto screensaver = std::make_unique<DummyTestScreen>();

    auto* chat_list_ptr = chat_list.get();

    navigator.setScreens(std::move(login), std::move(chat_list), nullptr);
    navigator.setScreensaver(std::move(screensaver));

    navigator.showChatList();
    navigator.lockScreen();
    navigator.lockScreen();
    ASSERT_TRUE(navigator.isLocked());

    navigator.unlockScreen();
    navigator.unlockScreen();
    ASSERT_FALSE(navigator.isLocked());
    ASSERT_EQ(chat_list_ptr, navigator.currentScreen());
}

TEST(screen_navigator_tracks_and_clears_active_conversation_id) {
    ScreenNavigator navigator;
    auto login = std::make_unique<DummyTestScreen>();
    auto chat_list = std::make_unique<DummyTestScreen>();
    auto conv = std::make_unique<ConversationScreen>(ChatId(101), "Default");

    auto* conv_ptr = conv.get();
    navigator.setScreens(std::move(login), std::move(chat_list), std::move(conv));

    ASSERT_FALSE(navigator.activeConversationId().has_value());

    navigator.openConversation(ChatId(777), "Alice");
    ASSERT_TRUE(navigator.activeConversationId().has_value());
    ASSERT_EQ(777, navigator.activeConversationId()->value());
    ASSERT_EQ(777, conv_ptr->chatId().value());

    navigator.showChatList();
    ASSERT_FALSE(navigator.activeConversationId().has_value());

    navigator.openConversation(ChatId(888), "Bob");
    ASSERT_TRUE(navigator.activeConversationId().has_value());
    ASSERT_EQ(888, navigator.activeConversationId()->value());

    navigator.showLogin();
    ASSERT_FALSE(navigator.activeConversationId().has_value());
}
