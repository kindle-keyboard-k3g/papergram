#include "test_framework.h"
#include "../src/ui/menu_item.h"
#include "../src/ui/menu_list.h"

TEST(menu_item_stores_label_and_executes_action) {
    bool executed = false;
    ui::MenuItem item(ui::MenuLabel("Refresh"), [&executed]() { executed = true; }, false);

    ASSERT_EQ(std::string("Refresh"), item.label().value());
    ASSERT_FALSE(item.isSeparatorBefore());

    item.execute();
    ASSERT_TRUE(executed);
}

TEST(menu_list_handles_empty_state) {
    ui::MenuList list;
    ASSERT_EQ(0U, list.count());
    ASSERT_FALSE(list.hasSelection());
}

TEST(menu_list_navigates_with_wraparound) {
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Item 1"), []() {}));
    list.append(ui::MenuItem(ui::MenuLabel("Item 2"), []() {}));
    list.append(ui::MenuItem(ui::MenuLabel("Item 3"), []() {}));

    ASSERT_EQ(3U, list.count());
    ASSERT_EQ(0U, list.selectedIndex());
    ASSERT_EQ(std::string("Item 1"), list.selectedItem().label().value());

    list.selectNext();
    ASSERT_EQ(1U, list.selectedIndex());

    list.selectNext();
    ASSERT_EQ(2U, list.selectedIndex());

    // Wrap around to 0
    list.selectNext();
    ASSERT_EQ(0U, list.selectedIndex());

    // Wrap around backward to 2
    list.selectPrevious();
    ASSERT_EQ(2U, list.selectedIndex());
}

TEST(menu_list_clear_resets_state) {
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Item 1"), []() {}));
    list.clear();
    ASSERT_EQ(0U, list.count());
    ASSERT_FALSE(list.hasSelection());
}
