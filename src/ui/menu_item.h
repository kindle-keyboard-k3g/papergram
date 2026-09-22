#ifndef KINDLE_UI_MENU_ITEM_H
#define KINDLE_UI_MENU_ITEM_H

#include "menu_label.h"
#include <functional>

namespace ui {

/**
 * @brief Associates a menu label with an optional executable action.
 */
class MenuItem {
public:
    /** @brief Callable invoked when the item is selected. */
    using Action = std::function<void()>;

    /**
     * @brief Creates a menu item.
     * @param label Display label for the item.
     * @param action Callback executed by execute(), when non-empty.
     * @param separator_before Whether to draw a separator before this item.
     */
    MenuItem(MenuLabel label, Action action, bool separator_before = false);

    /** @brief Invokes the item action when one is configured. */
    void execute() const;

    /**
     * @brief Returns the item's display label.
     * @return Stored menu label.
     */
    const MenuLabel& label() const;

    /**
     * @brief Reports whether a separator precedes the item.
     * @return True when a separator should be rendered first.
     */
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
