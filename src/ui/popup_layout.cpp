#include "popup_layout.h"

#include <algorithm>
#include <limits>

namespace ui {

BoundingBox PopupLayout::calculateBounds(std::size_t index,
                                         PopupPosition position) const {
    const int left = clampHorizontal(LEFT_MARGIN);
    const int right = clampHorizontal(RIGHT_MARGIN);
    const int top = clampVertical(topCoordinate(index, position));
    const int bottom = clampVertical(bottomCoordinate(index, position));
    return BoundingBox(left, top, right, bottom);
}

BoundingBox PopupLayout::calculateDamageArea(std::size_t count,
                                             PopupPosition position) const {
    if (count == 0U) {
        return BoundingBox(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    }
    const BoundingBox newest = calculateBounds(0U, position);
    const BoundingBox oldest = calculateBounds(count - 1U, position);
    const int top = std::min(newest.top(), oldest.top());
    const int bottom = std::max(newest.bottom(), oldest.bottom());
    return BoundingBox(newest.left(), top, newest.right(), bottom);
}

int PopupLayout::clampHorizontal(int coordinate) {
    return std::max(0, std::min(SCREEN_WIDTH - 1, coordinate));
}

int PopupLayout::clampVertical(long long coordinate) {
    const long long maximum = static_cast<long long>(SCREEN_HEIGHT - 1);
    return static_cast<int>(std::max(0LL, std::min(maximum, coordinate)));
}

long long PopupLayout::stackOffset(std::size_t index) {
    const long long maximum = std::numeric_limits<long long>::max();
    const std::size_t safe_index = static_cast<std::size_t>(maximum) /
                                   static_cast<std::size_t>(STACK_OFFSET);
    if (index > safe_index) {
        return maximum;
    }
    return static_cast<long long>(index) * STACK_OFFSET;
}

long long PopupLayout::topCoordinate(std::size_t index,
                                     PopupPosition position) {
    const long long offset = stackOffset(index);
    if (position == PopupPosition::TOP) {
        const long long maximum = std::numeric_limits<long long>::max();
        if (offset > maximum - TOP_ANCHOR) {
            return maximum;
        }
        return TOP_ANCHOR + offset;
    }
    return static_cast<long long>(BOTTOM_ANCHOR - CARD_HEIGHT) - offset;
}

long long PopupLayout::bottomCoordinate(std::size_t index,
                                        PopupPosition position) {
    const long long offset = stackOffset(index);
    if (position == PopupPosition::TOP) {
        const long long maximum = std::numeric_limits<long long>::max();
        if (offset > maximum - TOP_ANCHOR - CARD_HEIGHT) {
            return maximum;
        }
        return TOP_ANCHOR + CARD_HEIGHT + offset;
    }
    return static_cast<long long>(BOTTOM_ANCHOR) - offset;
}

} // namespace ui
