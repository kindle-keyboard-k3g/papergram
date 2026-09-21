#include "toast_notification.h"

#include "../graphics/bitmap_font.h"

#include <algorithm>

namespace ui {

ToastNotification::ToastNotification(const BoundingBox& bounds,
                                     const std::string& title,
                                     const std::vector<std::string>& lines)
    : data_{bounds, title, lines} {}

void ToastNotification::setBounds(const BoundingBox& bounds) {
    data_.bounds = bounds;
}

void ToastNotification::render(Canvas& canvas) const {
    renderBackground(canvas);
    renderBorder(canvas);
    renderTitle(canvas);
    renderLines(canvas);
}

const BoundingBox& ToastNotification::bounds() const {
    return data_.bounds;
}

void ToastNotification::renderBackground(Canvas& canvas) const {
    canvas.fillRect(data_.bounds, GrayscaleColor::LIGHT_GRAY);
}

void ToastNotification::renderBorder(Canvas& canvas) const {
    canvas.drawRect(data_.bounds, GrayscaleColor::BLACK);
    if (data_.bounds.width() <= 4 || data_.bounds.height() <= 4) return;
    const BoundingBox inner(data_.bounds.left() + 2, data_.bounds.top() + 2,
                            data_.bounds.right() - 2, data_.bounds.bottom() - 2);
    canvas.drawRect(inner, GrayscaleColor::DARK_GRAY);
}

void ToastNotification::renderTitle(Canvas& canvas) const {
    const int x = data_.bounds.left() + 12;
    const int y = data_.bounds.top() + 8;
    const std::size_t maximum_characters = maximumCharacters(data_.bounds, x);
    renderText(canvas, data_.title, maximum_characters, x, y);
}

void ToastNotification::renderLines(Canvas& canvas) const {
    int y = data_.bounds.top() + 30;
    for (std::size_t index = 0U;
         index < data_.lines.size() && fitsLine(data_.bounds, y); ++index) {
        const int x = data_.bounds.left() + 12;
        const std::size_t maximum_characters = maximumCharacters(data_.bounds, x);
        renderText(canvas, data_.lines[index], maximum_characters, x, y);
        y += 18;
    }
}

void ToastNotification::renderText(Canvas& canvas, const std::string& text,
                                   std::size_t maximum_characters, int x,
                                   int y) const {
    if (text.empty() || !canRenderText(data_.bounds, maximum_characters, x, y)) return;
    const std::size_t character_count = std::min(maximum_characters, text.size());
    for (std::size_t index = 0U; index < character_count; ++index) {
        const int character_x = x + static_cast<int>(index) *
                                      BitmapFont::CHARACTER_WIDTH;
        canvas.blitChar(ScreenCoordinate(character_x, y), text[index],
                        GrayscaleColor::BLACK);
    }
}

std::size_t ToastNotification::maximumCharacters(const BoundingBox& bounds,
                                                 int x) {
    const int right = std::min(bounds.right(), static_cast<int>(Canvas::WIDTH) - 1);
    if (x > right) return 0U;
    return static_cast<std::size_t>(right - x + 1) /
           BitmapFont::CHARACTER_WIDTH;
}

bool ToastNotification::canRenderText(const BoundingBox& bounds,
                                      std::size_t character_count, int x,
                                      int y) {
    if (character_count == 0U) return false;
    const int bottom = std::min(bounds.bottom(), static_cast<int>(Canvas::HEIGHT) - 1);
    const int right = std::min(bounds.right(), static_cast<int>(Canvas::WIDTH) - 1);
    const int last_row = y + static_cast<int>(BitmapFont::CHARACTER_HEIGHT) - 1;
    return x >= bounds.left() && x <= right && y >= bounds.top() &&
           last_row <= bottom && character_count <= maximumCharacters(bounds, x);
}

bool ToastNotification::fitsLine(const BoundingBox& bounds, int y) {
    const int bottom = std::min(bounds.bottom(), static_cast<int>(Canvas::HEIGHT) - 1);
    const int last_row = y + static_cast<int>(BitmapFont::CHARACTER_HEIGHT) - 1;
    return y >= bounds.top() && last_row <= bottom;
}

} // namespace ui
