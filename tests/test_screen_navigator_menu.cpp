#include "test_framework.h"
#include "../src/ui/screen_navigator.h"
#include "../src/graphics/canvas.h"

namespace {
class DummyMenuScreen : public IScreen {
public:
    int render_count = 0;
    int input_count = 0;

    void render(Canvas&) override { ++render_count; }
    void handleInput(const InputEvent&) override { ++input_count; }
    void onEnter() override {}
    void onExit() override {}
    std::vector<ui::MenuItem> contextualMenuItems() override {
        std::vector<ui::MenuItem> items;
        items.emplace_back(ui::MenuLabel("Context Item"), []() {});
        return items;
    }
};
}

TEST(screen_navigator_key_menu_toggles_kindle_menu) {
    ScreenNavigator navigator;
    auto screen = std::make_unique<DummyMenuScreen>();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    ASSERT_FALSE(navigator.isMenuOpen());

    InputEvent menu_ev{KeyCode::KEY_MENU, true};
    navigator.handleInput(menu_ev);
    ASSERT_TRUE(navigator.isMenuOpen());

    navigator.handleInput(menu_ev);
    ASSERT_FALSE(navigator.isMenuOpen());
}

TEST(screen_navigator_swallows_underlying_input_while_menu_is_open) {
    ScreenNavigator navigator;
    auto screen = std::make_unique<DummyMenuScreen>();
    auto* screen_ptr = screen.get();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    navigator.handleInput(InputEvent{KeyCode::KEY_MENU, true});
    ASSERT_TRUE(navigator.isMenuOpen());

    // Send a typing key
    navigator.handleInput(InputEvent{KeyCode::KEY_A, true});
    // Screen should NOT have received input
    ASSERT_EQ(0, screen_ptr->input_count);
}

TEST(screen_navigator_renders_menu_overlay_when_open) {
    Canvas canvas;
    ScreenNavigator navigator;
    auto screen = std::make_unique<DummyMenuScreen>();
    auto* screen_ptr = screen.get();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    navigator.handleInput(InputEvent{KeyCode::KEY_MENU, true});
    navigator.render(canvas);

    ASSERT_EQ(1, screen_ptr->render_count);
    ASSERT_TRUE(navigator.isMenuOpen());
}

TEST(screen_navigator_exit_menu_opens_confirmation_dialog) {
    ScreenNavigator navigator;
    auto screen = std::make_unique<DummyMenuScreen>();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    ASSERT_FALSE(navigator.isConfirmDialogOpen());

    // Open menu
    navigator.handleInput(InputEvent{KeyCode::KEY_MENU, true});
    ASSERT_TRUE(navigator.isMenuOpen());

    // Navigate to "Exit Papergram" (up 1 from index 0 wraps to last item)
    navigator.handleInput(InputEvent{KeyCode::KEY_UP, true});
    navigator.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    // Menu should be closed and confirmation dialog should be open
    ASSERT_FALSE(navigator.isMenuOpen());
    ASSERT_TRUE(navigator.isConfirmDialogOpen());
}

TEST(screen_navigator_confirm_exit_triggers_exit_callback) {
    ScreenNavigator navigator;
    bool exit_called = false;
    navigator.setExitCallback([&exit_called]() {
        exit_called = true;
    });

    auto screen = std::make_unique<DummyMenuScreen>();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    // Open menu and select Exit
    navigator.handleInput(InputEvent{KeyCode::KEY_MENU, true});
    navigator.handleInput(InputEvent{KeyCode::KEY_UP, true});
    navigator.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    ASSERT_TRUE(navigator.isConfirmDialogOpen());
    ASSERT_FALSE(exit_called);

    // Dialog defaults to Cancel. Press RIGHT to select Exit/Confirm.
    navigator.handleInput(InputEvent{KeyCode::KEY_RIGHT, true});
    // Press ENTER to confirm
    navigator.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    ASSERT_FALSE(navigator.isConfirmDialogOpen());
    ASSERT_TRUE(exit_called);
}

TEST(screen_navigator_cancel_exit_dismisses_dialog_without_exiting) {
    ScreenNavigator navigator;
    bool exit_called = false;
    navigator.setExitCallback([&exit_called]() {
        exit_called = true;
    });

    auto screen = std::make_unique<DummyMenuScreen>();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    // Open menu and select Exit
    navigator.handleInput(InputEvent{KeyCode::KEY_MENU, true});
    navigator.handleInput(InputEvent{KeyCode::KEY_UP, true});
    navigator.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    ASSERT_TRUE(navigator.isConfirmDialogOpen());

    // Dialog defaults to Cancel. Press ENTER immediately to cancel.
    navigator.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    ASSERT_FALSE(navigator.isConfirmDialogOpen());
    ASSERT_FALSE(exit_called);
}

TEST(screen_navigator_renders_confirmation_dialog_overlay_when_open) {
    Canvas canvas;
    ScreenNavigator navigator;
    auto screen = std::make_unique<DummyMenuScreen>();
    auto* screen_ptr = screen.get();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    // Open menu and select Exit
    navigator.handleInput(InputEvent{KeyCode::KEY_MENU, true});
    navigator.handleInput(InputEvent{KeyCode::KEY_UP, true});
    navigator.handleInput(InputEvent{KeyCode::KEY_ENTER, true});

    ASSERT_TRUE(navigator.isConfirmDialogOpen());

    navigator.render(canvas);
    ASSERT_EQ(1, screen_ptr->render_count);
}
