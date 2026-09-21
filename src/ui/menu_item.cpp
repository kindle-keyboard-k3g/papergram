#include "menu_item.h"

namespace ui {

MenuItem::MenuItem(MenuLabel label, Action action, bool separator_before)
    : data_{std::move(label), std::move(action), separator_before} {}

void MenuItem::execute() const {
    if (data_.action) {
        data_.action();
    }
}

const MenuLabel& MenuItem::label() const {
    return data_.label;
}

bool MenuItem::isSeparatorBefore() const {
    return data_.separator_before;
}

} // namespace ui
