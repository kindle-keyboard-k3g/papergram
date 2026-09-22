#ifndef KINDLE_UI_MENU_LIST_H
#define KINDLE_UI_MENU_LIST_H

#include "menu_item.h"
#include <vector>
#include <cstddef>

namespace ui {

/**
 * @brief Owns menu items and manages a wrapping selection cursor.
 */
class MenuList {
public:
    /** @brief Creates an empty menu list with no selected item. */
    MenuList();

    /**
     * @brief Appends an item to the list.
     * @param item Item to copy into the list.
     */
    void append(const MenuItem& item);

    /** @brief Removes every item and resets the selection index. */
    void clear();

    /** @brief Moves the selection forward, wrapping at the end. */
    void selectNext();

    /** @brief Moves the selection backward, wrapping at the beginning. */
    void selectPrevious();

    /** @brief Sets the selection to the first item when one exists. */
    void resetSelection();

    /**
     * @brief Returns the current selection index.
     * @return Zero-based selected index.
     */
    std::size_t selectedIndex() const;

    /**
     * @brief Returns the currently selected item.
     * @return Selected menu item.
     * @throws std::out_of_range If the list is empty.
     */
    const MenuItem& selectedItem() const;

    /**
     * @brief Returns an item by index.
     * @param index Zero-based item index.
     * @return Menu item at index.
     * @throws std::out_of_range If index is outside the list.
     */
    const MenuItem& at(std::size_t index) const;

    /**
     * @brief Returns the number of items.
     * @return Item count.
     */
    std::size_t count() const;

    /**
     * @brief Reports whether at least one item can be selected.
     * @return True when the list is non-empty.
     */
    bool hasSelection() const;

private:
    std::vector<MenuItem> items_;
    std::size_t selected_index_ = 0U;
};

} // namespace ui

#endif
