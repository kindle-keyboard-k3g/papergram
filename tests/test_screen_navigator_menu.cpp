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
