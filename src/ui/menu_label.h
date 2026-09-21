#ifndef KINDLE_UI_MENU_LABEL_H
#define KINDLE_UI_MENU_LABEL_H

#include <string>

namespace ui {

class MenuLabel {
public:
    explicit MenuLabel(const std::string& text);

    const std::string& value() const;

private:
    std::string value_;
};

} // namespace ui

#endif
