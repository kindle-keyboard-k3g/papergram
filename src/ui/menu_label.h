#ifndef KINDLE_UI_MENU_LABEL_H
#define KINDLE_UI_MENU_LABEL_H

#include <string>

namespace ui {

/**
 * @brief Stores a validated, display-ready menu label.
 */
class MenuLabel {
public:
    /**
     * @brief Creates a label after trimming surrounding whitespace.
     * @param text Text to normalize and store.
     * @throws std::invalid_argument If the trimmed text is empty.
     */
    explicit MenuLabel(const std::string& text);

    /**
     * @brief Returns the normalized label text.
     * @return Stored menu label.
     */
    const std::string& value() const;

private:
    std::string value_;
};

} // namespace ui

#endif
