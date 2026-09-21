#include "test_framework.h"
#include "mocks/mock_network_transport.h"
#include "../src/domain/value_objects.h"
#include "../src/graphics/canvas.h"
#include "../src/mtproto/telegram_client.h"
#include "../src/mtproto/tl_codec.h"
#include "../src/ui/login_screen.h"
#include "../src/ui/screen_navigator.h"

#include <cstdio>
#include <vector>

namespace {

const char* TEST_SESSION_LOGIN = "/tmp/test_session_login.dat";

void cleanupSession() {
    std::remove(TEST_SESSION_LOGIN);
}

void inputString(LoginScreen& screen, const std::string& str) {
    for (char c : str) {
        if (c >= '0' && c <= '9') {
            screen.handleInput(InputEvent{static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_0) + (c - '0')), true});
        } else if (c >= 'a' && c <= 'z') {
            screen.handleInput(InputEvent{static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + (c - 'a')), true});
        }
    }
}

std::vector<std::uint8_t> makeAuthResponse() {
    mtproto::TlWriter authWriter;
    authWriter.write_int32(0xcd05da1b); // auth.authorization
    std::vector<std::uint8_t> dummy_key(256, 0x42);
    authWriter.write_bytes(dummy_key);
    authWriter.write_int64(0x11223344ULL);
    authWriter.write_int64(0x55667788ULL);
    authWriter.write_int32(1);
    return authWriter.data();
}

} // namespace

TEST(login_screen_phone_step_renders_keyboard_toast) {
    cleanupSession();
    MockNetworkTransport transport;
    mtproto::TelegramClient client(transport, TEST_SESSION_LOGIN);
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);
    Canvas canvas;

    screen.onEnter();
    screen.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(40, 270, 560, 420)));
    cleanupSession();
}

TEST(login_screen_phone_backspace_preserves_plus_prefix) {
    cleanupSession();
    MockNetworkTransport transport;
    mtproto::TelegramClient client(transport, TEST_SESSION_LOGIN);
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);
    Canvas canvas;

    screen.onEnter();
    screen.handleInput(InputEvent{KeyCode::KEY_BACKSPACE, true});
    screen.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(50, 162, 57, 173)));
    cleanupSession();
}

TEST(login_screen_submits_phone_and_transitions_to_code_on_success) {
    cleanupSession();
    MockNetworkTransport transport;
    mtproto::TlWriter codeWriter;
    codeWriter.write_int32(0x86b); // auth.sentCode
    transport.setResponse(codeWriter.data());

    mtproto::TelegramClient client(transport, TEST_SESSION_LOGIN);
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);

    screen.onEnter();
    inputString(screen, "1234567890"); // buffer is "+1234567890"

    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    Canvas canvas;
    screen.render(canvas);

    // Should now be in code step: buffer empty, code tips rendered
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(40, 270, 560, 420)));
    ASSERT_FALSE(canvas.hasNonWhitePixel(BoundingBox(50, 162, 57, 173))); // buffer cleared
    cleanupSession();
}

TEST(login_screen_shows_error_toast_on_network_or_phone_failure) {
    cleanupSession();
    MockNetworkTransport transport;
    transport.setSuccess(false);

    mtproto::TelegramClient client(transport, TEST_SESSION_LOGIN);
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);

    screen.onEnter();
    inputString(screen, "1234567890");
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    Canvas canvas;
    screen.render(canvas);

    // Should show error feedback on failure
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(40, 240, 560, 440)));
    cleanupSession();
}

TEST(login_screen_submits_code_and_navigates_to_chat_list_on_success) {
    cleanupSession();
    MockNetworkTransport transport;
    mtproto::TlWriter codeWriter;
    codeWriter.write_int32(0x86b);
    transport.setResponse(codeWriter.data());

    mtproto::TelegramClient client(transport, TEST_SESSION_LOGIN);
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);

    screen.onEnter();
    inputString(screen, "1234567890");
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    // Now in code step, set up auth response for signIn
    transport.setResponse(makeAuthResponse());
    inputString(screen, "12345");
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    ASSERT_TRUE(client.isAuthorized());
    cleanupSession();
}

TEST(login_screen_transitions_to_2fa_step_when_password_needed) {
    cleanupSession();
    MockNetworkTransport transport;
    mtproto::TlWriter codeWriter;
    codeWriter.write_int32(0x86b);
    transport.setResponse(codeWriter.data());

    mtproto::TelegramClient client(transport, TEST_SESSION_LOGIN);
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);

    screen.onEnter();
    inputString(screen, "1234567890");
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    // In code step, respond with password needed constructor (0x300)
    mtproto::TlWriter pwdNeededWriter;
    pwdNeededWriter.write_int32(0x300);
    transport.setResponse(pwdNeededWriter.data());

    inputString(screen, "12345");
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    Canvas canvas;
    screen.render(canvas);

    // Should now be in 2FA step
    ASSERT_FALSE(client.isAuthorized());
    ASSERT_TRUE(client.isPasswordNeeded());
    cleanupSession();
}

TEST(login_screen_submits_password_and_authenticates) {
    cleanupSession();
    MockNetworkTransport transport;
    mtproto::TlWriter codeWriter;
    codeWriter.write_int32(0x86b);
    transport.setResponse(codeWriter.data());

    mtproto::TelegramClient client(transport, TEST_SESSION_LOGIN);
    ScreenNavigator navigator;
    LoginScreen screen(client, navigator);

    screen.onEnter();
    inputString(screen, "1234567890");
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    mtproto::TlWriter pwdNeededWriter;
    pwdNeededWriter.write_int32(0x300);
    transport.setResponse(pwdNeededWriter.data());
    inputString(screen, "12345");
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    // Now in 2FA step, set up auth response for checkPassword
    transport.setResponse(makeAuthResponse());
    inputString(screen, "secret");
    screen.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    ASSERT_TRUE(client.isAuthorized());
    cleanupSession();
}
