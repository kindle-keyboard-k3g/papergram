#include "test_framework.h"

#include "../src/graphics/canvas.h"
#include "../src/mtproto/telegram_client.h"
#include "../src/ui/login_screen.h"
#include "../src/ui/screen_navigator.h"

class LoginTestTransport : public INetworkTransport {
public:
    bool post(const std::string&, const std::vector<uint8_t>&,
              std::vector<uint8_t>&) override {
        return true;
    }
};

TEST(login_screen_phone_step_renders_keyboard_toast) {
    LoginTestTransport transport;
    mtproto::TelegramClient client(transport, "/tmp/test_session_login.dat");
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);
    Canvas canvas;

    screen.onEnter();
    screen.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(40, 270, 560, 420)));
}

TEST(login_screen_phone_backspace_preserves_plus_prefix) {
    LoginTestTransport transport;
    mtproto::TelegramClient client(transport, "/tmp/test_session_login.dat");
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);
    Canvas canvas;

    screen.onEnter();
    screen.handleInput(InputEvent{KeyCode::KEY_BACKSPACE, true});
    screen.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(50, 162, 57, 173)));
}

TEST(login_screen_enter_renders_code_toast_and_resets_buffer) {
    LoginTestTransport transport;
    mtproto::TelegramClient client(transport, "/tmp/test_session_login.dat");
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);
    Canvas canvas;

    screen.onEnter();
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});
    screen.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(40, 270, 560, 420)));
    ASSERT_EQ(GrayscaleColor::BLACK,
              canvas.pixelAt(ScreenCoordinate(55, 279)));
    ASSERT_FALSE(canvas.hasNonWhitePixel(BoundingBox(50, 162, 57, 173)));
}
