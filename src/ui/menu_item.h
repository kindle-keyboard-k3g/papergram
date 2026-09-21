#ifndef KINDLE_UI_MENU_ITEM_H
#define KINDLE_UI_MENU_ITEM_H

#include "menu_label.h"
#include <functional>

namespace ui {

class MenuItem {
public:
    using Action = std::function<void()>;

    MenuItem(MenuLabel label, Action action, bool separator_before = false);

    void execute() const;
    const MenuLabel& label() const;
    bool isSeparatorBefore() const;

private:
    struct ItemData {
        MenuLabel label;
        Action action;
        bool separator_before;
    };

    ItemData data_;
};

} // namespace ui

#endif
