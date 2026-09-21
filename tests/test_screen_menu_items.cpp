#include "test_framework.h"
#include "../src/ui/login_screen.h"
#include "../src/ui/chat_list_screen.h"
#include "../src/ui/conversation_screen.h"
#include "../src/ui/screen_navigator.h"
#include "../src/mtproto/telegram_client.h"
#include "../src/hal/network_transport.h"

namespace {
class MenuTestTransport : public INetworkTransport {
public:
    bool post(const std::string&, const std::vector<uint8_t>&,
              std::vector<uint8_t>&) override {
        return true;
    }
};
}

TEST(login_screen_provides_contextual_menu_items) {
    MenuTestTransport transport;
    mtproto::TelegramClient client(transport, "/tmp/test_session_login.dat");
    ScreenNavigator navigator;
    LoginScreen login(client, navigator);

    auto items = login.contextualMenuItems();
    ASSERT_TRUE(items.size() >= 2U);
    ASSERT_EQ(std::string("Reset Form"), items[0].label().value());
    ASSERT_EQ(std::string("Keyboard Tips"), items[1].label().value());
}

TEST(chat_list_screen_provides_contextual_menu_items) {
    MenuTestTransport transport;
    mtproto::TelegramClient client(transport, "/tmp/test_session_chat.dat");
    ScreenNavigator navigator;
    ChatListScreen chat_list(client, navigator);

    auto items = chat_list.contextualMenuItems();
    ASSERT_TRUE(items.size() >= 2U);
    ASSERT_EQ(std::string("Refresh Chats"), items[0].label().value());
    ASSERT_EQ(std::string("Mark All Read"), items[1].label().value());
}

TEST(conversation_screen_provides_contextual_menu_items) {
    MenuTestTransport transport;
    mtproto::TelegramClient client(transport, "/tmp/test_session_conv.dat");
    ScreenNavigator navigator;
    ConversationScreen conv(client, navigator);

    auto items = conv.contextualMenuItems();
    ASSERT_TRUE(items.size() >= 3U);
    ASSERT_EQ(std::string("Refresh History"), items[0].label().value());
    ASSERT_EQ(std::string("Clear Input"), items[1].label().value());
    ASSERT_EQ(std::string("Back to Chats"), items[2].label().value());
}

