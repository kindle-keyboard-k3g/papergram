#include "menu_label.h"
#include <stdexcept>

namespace {
std::string trim(const std::string& str) {
    std::size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    std::size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
} // namespace

namespace ui {

MenuLabel::MenuLabel(const std::string& text) : value_(trim(text)) {
    if (value_.empty()) {
        throw std::invalid_argument("Menu label cannot be empty or whitespace");
    }
}

const std::string& MenuLabel::value() const {
    return value_;
}

} // namespace ui
