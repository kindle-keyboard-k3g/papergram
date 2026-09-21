#include "test_framework.h"
#include "../src/graphics/canvas.h"
#include "../src/ui/screensaver_screen.h"
#include "../src/ui/screen_navigator.h"

namespace {
class MockScreen : public IScreen {
public:
    int enter_count = 0;
    int exit_count = 0;
    int input_count = 0;

    void render(Canvas&) override {}
    void handleInput(const InputEvent&) override { ++input_count; }
    void onEnter() override { ++enter_count; }
    void onExit() override { ++exit_count; }
};
}

TEST(screensaver_screen_renders_visual_elements_on_canvas) {
    ScreensaverScreen screen(nullptr);
    Canvas canvas;
    screen.render(canvas);

    ASSERT_EQ(0x00, canvas.pixelAt(ScreenCoordinate(16, 16)).value());
    ASSERT_EQ(0x00, canvas.pixelAt(ScreenCoordinate(20, 20)).value());
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(100, 260, 499, 520)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(256, 310, 350, 330)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(160, 720, 450, 760)));
}

TEST(screensaver_screen_ignores_regular_keystrokes) {
    ScreenNavigator navigator;
    auto mock = std::make_unique<MockScreen>();
    auto* mock_ptr = mock.get();
    navigator.setScreens(std::move(mock), nullptr, nullptr);
    navigator.setScreensaver(std::make_unique<ScreensaverScreen>(&navigator));
    navigator.lockScreen();

    ASSERT_TRUE(navigator.isLocked());

    InputEvent key_a{KeyCode::KEY_A, true};
    InputEvent key_enter{KeyCode::KEY_ENTER, true};
    InputEvent key_down{KeyCode::KEY_DOWN, true};
    InputEvent key_backspace{KeyCode::KEY_BACKSPACE, true};

    navigator.handleInput(key_a);
    navigator.handleInput(key_enter);
    navigator.handleInput(key_down);
    navigator.handleInput(key_backspace);

    ASSERT_TRUE(navigator.isLocked());
    ASSERT_EQ(0, mock_ptr->input_count);
}

TEST(screensaver_screen_wakes_on_key_power) {
    ScreenNavigator navigator;
    auto mock = std::make_unique<MockScreen>();
    auto* mock_ptr = mock.get();
    navigator.setScreens(std::move(mock), nullptr, nullptr);
    navigator.setScreensaver(std::make_unique<ScreensaverScreen>(&navigator));
    navigator.showLogin();
    ASSERT_EQ(1, mock_ptr->enter_count);

    navigator.lockScreen();
    ASSERT_TRUE(navigator.isLocked());

    InputEvent release_power{KeyCode::KEY_POWER, false};
    navigator.handleInput(release_power);
    ASSERT_TRUE(navigator.isLocked());

    InputEvent press_power{KeyCode::KEY_POWER, true};
    navigator.handleInput(press_power);
    ASSERT_FALSE(navigator.isLocked());
    ASSERT_EQ(2, mock_ptr->enter_count);
}
