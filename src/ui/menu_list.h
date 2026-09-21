#ifndef KINDLE_UI_MENU_LIST_H
#define KINDLE_UI_MENU_LIST_H

#include "menu_item.h"
#include <vector>
#include <cstddef>

namespace ui {

class MenuList {
public:
    MenuList();

    void append(const MenuItem& item);
    void clear();

    void selectNext();
    void selectPrevious();
    void resetSelection();

    std::size_t selectedIndex() const;
    const MenuItem& selectedItem() const;
    const MenuItem& at(std::size_t index) const;
    std::size_t count() const;
    bool hasSelection() const;

private:
    std::vector<MenuItem> items_;
    std::size_t selected_index_ = 0U;
};

} // namespace ui

#endif
