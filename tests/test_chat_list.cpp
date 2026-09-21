#include "test_framework.h"

#include "../src/domain/chat.h"
#include "../src/graphics/canvas.h"
#include "../src/mtproto/telegram_client.h"
#include "../src/ui/chat_list_screen.h"
#include "../src/ui/screen_navigator.h"
#include "mocks/mock_network_transport.h"

#include "../src/mtproto/tl_codec.h"

class DummyNetworkTransport : public INetworkTransport {
public:
    bool post(const std::string&, const std::vector<uint8_t>&, std::vector<uint8_t>& response) override {
        mtproto::TlWriter writer;
        writer.write_int32(0x105);
        writer.write_int32(2);
        writer.write_int64(101);
        writer.write_string("Tech Chat");
        writer.write_int32(1);
        writer.write_int64(102);
        writer.write_string("Kindle Dev");
        writer.write_int32(0);
        response = writer.data();
        return true;
    }
};

TEST(chat_list_screen_populates_dialogs_and_navigates) {
    DummyNetworkTransport transport;
    mtproto::TelegramClient client(transport, "/tmp/test_session_chat.dat");
    ScreenNavigator navigator;
    ChatListScreen screen(client, navigator);

    screen.onEnter();
    Canvas canvas;
    screen.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(0, 0, 600, 30)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(10, 40, 590, 85)));

    InputEvent downEvent{KeyCode::KEY_DOWN, true};
    screen.handleInput(downEvent);

    InputEvent enterEvent{KeyCode::KEY_ENTER, true};
    screen.handleInput(enterEvent);
}
