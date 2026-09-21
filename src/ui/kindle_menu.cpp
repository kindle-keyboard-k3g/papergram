#include "kindle_menu.h"
#include "../graphics/bitmap_font.h"

namespace {
constexpr int MENU_WIDTH = 460;
constexpr int MENU_LEFT = 70;
constexpr int HEADER_HEIGHT = 38;
constexpr int ITEM_HEIGHT = 36;
constexpr int SEPARATOR_HEIGHT = 10;
constexpr int PADDING_BOTTOM = 14;
} // namespace

namespace ui {

KindleMenu::KindleMenu() = default;

void KindleMenu::open(const MenuList& items) {
    state_.items = items;
    state_.items.resetSelection();
    state_.open = true;
}

void KindleMenu::close() {
    state_.open = false;
}

bool KindleMenu::isOpen() const {
    return state_.open;
}

bool KindleMenu::handleInput(const InputEvent& event) {
    if (!state_.open || !event.pressed) return false;
    if (event.code == KeyCode::KEY_BACK || event.code == KeyCode::KEY_MENU) {
        close();
        return true;
    }
    if (event.code == KeyCode::KEY_UP) {
        state_.items.selectPrevious();
        return true;
    }
    if (event.code == KeyCode::KEY_DOWN) {
        state_.items.selectNext();
        return true;
    }
    if (event.code == KeyCode::KEY_ENTER && state_.items.hasSelection()) {
        const MenuItem& selected = state_.items.selectedItem();
        close();
        selected.execute();
        return true;
    }
    return true; // Consume other keystrokes while menu is open
}

BoundingBox KindleMenu::computeBounds(std::size_t item_count, bool has_separator) {
    int height = HEADER_HEIGHT + (static_cast<int>(item_count) * ITEM_HEIGHT) + PADDING_BOTTOM;
    if (has_separator) height += SEPARATOR_HEIGHT;
    int top = (800 - height) / 2;
    return BoundingBox(MENU_LEFT, top, MENU_LEFT + MENU_WIDTH, top + height);
}

void KindleMenu::render(Canvas& canvas) const {
    if (!state_.open || state_.items.count() == 0) return;
    bool has_sep = false;
    for (std::size_t i = 0; i < state_.items.count(); ++i) {
        if (state_.items.at(i).isSeparatorBefore()) has_sep = true;
    }
    BoundingBox box = computeBounds(state_.items.count(), has_sep);
    renderCard(canvas, box);
    renderTitle(canvas, box);
    renderItems(canvas, box.top() + HEADER_HEIGHT, box.left() + 10, box.right() - 10);
}

void KindleMenu::renderCard(Canvas& canvas, const BoundingBox& box) const {
    canvas.fillRect(box, GrayscaleColor::WHITE);
    canvas.drawRect(box, GrayscaleColor::BLACK);
    canvas.drawRect(BoundingBox(box.left() + 2, box.top() + 2, box.right() - 2, box.bottom() - 2),
                    GrayscaleColor::DARK_GRAY);
}

void KindleMenu::renderTitle(Canvas& canvas, const BoundingBox& box) const {
    canvas.fillRect(BoundingBox(box.left() + 3, box.top() + 3, box.right() - 3, box.top() + HEADER_HEIGHT - 4),
                    GrayscaleColor::LIGHT_GRAY);
    canvas.blitText(ScreenCoordinate(box.left() + 160, box.top() + 10),
                    "=== MENU ===", GrayscaleColor::BLACK);
    canvas.drawLine(ScreenCoordinate(box.left() + 3, box.top() + HEADER_HEIGHT - 3),
                    ScreenCoordinate(box.right() - 3, box.top() + HEADER_HEIGHT - 3),
                    GrayscaleColor::BLACK);
}

void KindleMenu::renderItems(Canvas& canvas, int start_y, int left_x, int right_x) const {
    int current_y = start_y;
    for (std::size_t i = 0; i < state_.items.count(); ++i) {
        if (state_.items.at(i).isSeparatorBefore()) {
            canvas.drawLine(ScreenCoordinate(left_x, current_y + 4),
                            ScreenCoordinate(right_x, current_y + 4),
                            GrayscaleColor::DARK_GRAY);
            current_y += SEPARATOR_HEIGHT;
        }
        renderItemRow(canvas, i, current_y, left_x, right_x);
        current_y += ITEM_HEIGHT;
    }
}

void KindleMenu::renderItemRow(Canvas& canvas, std::size_t index, int y, int left_x, int right_x) const {
    bool is_selected = (index == state_.items.selectedIndex());
    if (is_selected) {
        canvas.fillRect(BoundingBox(left_x, y + 2, right_x, y + ITEM_HEIGHT - 2),
                        GrayscaleColor::BLACK);
        std::string text = "> " + state_.items.at(index).label().value();
        canvas.blitText(ScreenCoordinate(left_x + 12, y + 10), text, GrayscaleColor::WHITE);
        return;
    }
    std::string text = "  " + state_.items.at(index).label().value();
    canvas.blitText(ScreenCoordinate(left_x + 12, y + 10), text, GrayscaleColor::BLACK);
}

} // namespace ui
