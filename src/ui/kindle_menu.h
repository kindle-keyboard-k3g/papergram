#ifndef KINDLE_UI_KINDLE_MENU_H
#define KINDLE_UI_KINDLE_MENU_H

#include "menu_list.h"
#include "../graphics/canvas.h"
#include "../hal/input_device.h"

namespace ui {

class KindleMenu {
public:
    KindleMenu();

    void open(const MenuList& items);
    void close();
    bool isOpen() const;

    bool handleInput(const InputEvent& event);
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
