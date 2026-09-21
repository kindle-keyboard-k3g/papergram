#include "test_framework.h"
#include "../src/ui/kindle_menu.h"
#include "../src/graphics/canvas.h"

TEST(kindle_menu_initial_state_is_closed) {
    ui::KindleMenu menu;
    ASSERT_FALSE(menu.isOpen());
}

TEST(kindle_menu_open_and_close) {
    ui::KindleMenu menu;
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Option 1"), []() {}));

    menu.open(list);
    ASSERT_TRUE(menu.isOpen());

    menu.close();
    ASSERT_FALSE(menu.isOpen());
}

TEST(kindle_menu_handles_navigation_and_execution) {
    ui::KindleMenu menu;
    bool action1_ran = false;
    bool action2_ran = false;
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Action 1"), [&]() { action1_ran = true; }));
    list.append(ui::MenuItem(ui::MenuLabel("Action 2"), [&]() { action2_ran = true; }));

    menu.open(list);

    // Down arrow moves to Action 2
    InputEvent down_ev{KeyCode::KEY_DOWN, true};
    ASSERT_TRUE(menu.handleInput(down_ev));

    // Enter executes Action 2 and closes menu
    InputEvent enter_ev{KeyCode::KEY_ENTER, true};
    ASSERT_TRUE(menu.handleInput(enter_ev));
    ASSERT_TRUE(action2_ran);
    ASSERT_FALSE(action1_ran);
    ASSERT_FALSE(menu.isOpen());
}

TEST(kindle_menu_dismiss_with_back_and_menu_key) {
    ui::KindleMenu menu;
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Option"), []() {}));

    menu.open(list);
    InputEvent back_ev{KeyCode::KEY_BACK, true};
    ASSERT_TRUE(menu.handleInput(back_ev));
    ASSERT_FALSE(menu.isOpen());

    menu.open(list);
    InputEvent menu_ev{KeyCode::KEY_MENU, true};
    ASSERT_TRUE(menu.handleInput(menu_ev));
    ASSERT_FALSE(menu.isOpen());
}

TEST(kindle_menu_renders_double_border_and_items_on_canvas) {
    Canvas canvas;
    ui::KindleMenu menu;
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Item 1"), []() {}));
    list.append(ui::MenuItem(ui::MenuLabel("Item 2"), []() {}, true)); // with separator

    menu.open(list);
    menu.render(canvas);

    // Verify canvas rendered without crash and menu remains open
    ASSERT_TRUE(menu.isOpen());
}
