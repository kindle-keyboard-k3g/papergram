#ifndef KINDLE_UI_POPUP_LAYOUT_H
#define KINDLE_UI_POPUP_LAYOUT_H

#include "popup_types.h"
#include "../domain/value_objects.h"

#include <cstddef>

namespace ui {

class PopupLayout {
public:
    static constexpr int SCREEN_WIDTH = 600;
    static constexpr int SCREEN_HEIGHT = 800;
    static constexpr int LEFT_MARGIN = 30;
    static constexpr int RIGHT_MARGIN = 570;
    static constexpr int CARD_HEIGHT = 68;
    static constexpr int STACK_OFFSET = 48;
    static constexpr int TOP_ANCHOR = 36;
    static constexpr int BOTTOM_ANCHOR = 760;

    BoundingBox calculateBounds(std::size_t index, PopupPosition position) const;
    BoundingBox calculateDamageArea(std::size_t count, PopupPosition position) const;

private:
    static int clampHorizontal(int coordinate);
    static int clampVertical(long long coordinate);
    static long long stackOffset(std::size_t index);
    static long long topCoordinate(std::size_t index, PopupPosition position);
    static long long bottomCoordinate(std::size_t index, PopupPosition position);
};

} // namespace ui

#endif
