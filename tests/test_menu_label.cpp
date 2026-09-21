#include "test_framework.h"
#include "../src/ui/menu_label.h"

#include <stdexcept>

TEST(menu_label_accepts_valid_string) {
    ui::MenuLabel label("Refresh Screen");
    ASSERT_EQ(std::string("Refresh Screen"), label.value());
}

TEST(menu_label_rejects_empty_or_whitespace_string) {
    bool rejected_empty = false;
    try {
        ui::MenuLabel label("");
    } catch (const std::invalid_argument&) {
        rejected_empty = true;
    }
    ASSERT_TRUE(rejected_empty);

    bool rejected_whitespace = false;
    try {
        ui::MenuLabel label("   ");
    } catch (const std::invalid_argument&) {
        rejected_whitespace = true;
    }
    ASSERT_TRUE(rejected_whitespace);
}


TEST(menu_label_trims_surrounding_whitespace) {
    ui::MenuLabel label("  Log Out  ");
    ASSERT_EQ(std::string("Log Out"), label.value());
}
