#include "menu_list.h"
#include <stdexcept>

namespace ui {

MenuList::MenuList() = default;

void MenuList::append(const MenuItem& item) {
    items_.push_back(item);
}

void MenuList::clear() {
    items_.clear();
    selected_index_ = 0U;
}

void MenuList::selectNext() {
    if (items_.empty()) return;
    selected_index_ = (selected_index_ + 1U) % items_.size();
}

void MenuList::selectPrevious() {
    if (items_.empty()) return;
    selected_index_ = (selected_index_ + items_.size() - 1U) % items_.size();
}

void MenuList::resetSelection() {
    selected_index_ = 0U;
}

std::size_t MenuList::selectedIndex() const {
    return selected_index_;
}

const MenuItem& MenuList::selectedItem() const {
    return at(selected_index_);
}

const MenuItem& MenuList::at(std::size_t index) const {
    if (index >= items_.size()) {
        throw std::out_of_range("Menu index out of range");
    }
    return items_[index];
}

std::size_t MenuList::count() const {
    return items_.size();
}

bool MenuList::hasSelection() const {
    return !items_.empty();
}

} // namespace ui
