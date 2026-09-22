#ifndef KINDLE_UI_KINDLE_MENU_H
#define KINDLE_UI_KINDLE_MENU_H

#include "menu_list.h"
#include "../graphics/canvas.h"
#include "../hal/input_device.h"

namespace ui {

/**
 * @brief Displays and navigates the application's contextual menu overlay.
 */
class KindleMenu {
public:
    /** @brief Creates a closed menu with no items. */
    KindleMenu();

    /**
     * @brief Opens the menu with a copied item list.
     * @param items Items to display and navigate.
     */
    void open(const MenuList& items);

    /** @brief Closes the menu without executing the selected item. */
    void close();

    /**
     * @brief Reports whether the menu is open.
     * @return True when the menu overlay is visible.
     */
    bool isOpen() const;

    /**
     * @brief Handles navigation, dismissal, and item activation input.
     * @param event Input event to process.
     * @return True when the event is consumed by the menu.
     */
    bool handleInput(const InputEvent& event);

    /**
     * @brief Renders the menu overlay when it contains items and is open.
     * @param canvas Destination canvas.
     */
    void render(Canvas& canvas) const;

private:
    struct MenuState {
        MenuList items;
        bool open = false;
    };

    MenuState state_;

    void renderCard(Canvas& canvas, const BoundingBox& box) const;
    void renderTitle(Canvas& canvas, const BoundingBox& box) const;
    void renderItems(Canvas& canvas, int start_y, int left_x, int right_x) const;
    void renderItemRow(Canvas& canvas, std::size_t index, int y, int left_x, int right_x) const;
    static BoundingBox computeBounds(std::size_t item_count, bool has_separator);
};

} // namespace ui

#endif
