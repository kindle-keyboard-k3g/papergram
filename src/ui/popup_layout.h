#ifndef KINDLE_UI_POPUP_LAYOUT_H
#define KINDLE_UI_POPUP_LAYOUT_H

#include "popup_types.h"
#include "../domain/value_objects.h"

#include <cstddef>

namespace ui {

/**
 * @brief Calculates popup card positions and redraw regions for the display.
 */
class PopupLayout {
public:
    /** @brief Display width used by popup layout calculations. */
    static constexpr int SCREEN_WIDTH = 600;
    /** @brief Display height used by popup layout calculations. */
    static constexpr int SCREEN_HEIGHT = 800;
    /** @brief Left edge shared by popup cards. */
    static constexpr int LEFT_MARGIN = 30;
    /** @brief Right edge shared by popup cards. */
    static constexpr int RIGHT_MARGIN = 570;
    /** @brief Height of each popup card. */
    static constexpr int CARD_HEIGHT = 68;
    /** @brief Vertical offset between stacked cards. */
    static constexpr int STACK_OFFSET = 48;
    /** @brief Top anchor coordinate. */
    static constexpr int TOP_ANCHOR = 36;
    /** @brief Bottom anchor coordinate. */
    static constexpr int BOTTOM_ANCHOR = 760;

    /**
     * @brief Calculates the bounds of one popup card.
     * @param index Zero-based visual stack index.
     * @param position Top or bottom anchor for the stack.
     * @return Clamped card bounds within the display.
     */
    BoundingBox calculateBounds(std::size_t index, PopupPosition position) const;

    /**
     * @brief Calculates the area affected by a popup collection redraw.
     * @param count Number of popup cards in the collection.
     * @param position Top or bottom anchor for the stack.
     * @return Damage bounds, or the full display when count is zero.
     */
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
