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
    renderText(canvas, clippedText(data_.bounds, data_.title, x), x, y);
}

void ToastNotification::renderLines(Canvas& canvas) const {
    int y = data_.bounds.top() + 30;
    for (std::size_t index = 0U;
         index < data_.lines.size() && fitsLine(data_.bounds, y); ++index) {
        const int x = data_.bounds.left() + 12;
        const std::string text = clippedText(data_.bounds, data_.lines[index], x);
        renderText(canvas, text, x, y);
        y += 18;
    }
}

void ToastNotification::renderText(Canvas& canvas, const std::string& text,
                                   int x, int y) const {
    if (!canRenderText(data_.bounds, text, x, y)) return;
    canvas.blitText(ScreenCoordinate(x, y), text, GrayscaleColor::BLACK);
}

std::string ToastNotification::clippedText(const BoundingBox& bounds,
                                           const std::string& text, int x) {
    return text.substr(0U, maximumCharacters(bounds, x));
}

std::size_t ToastNotification::maximumCharacters(const BoundingBox& bounds,
                                                 int x) {
    const int right = std::min(bounds.right(), static_cast<int>(Canvas::WIDTH) - 1);
    if (x > right) return 0U;
    return static_cast<std::size_t>(right - x + 1) /
           BitmapFont::CHARACTER_WIDTH;
}

bool ToastNotification::canRenderText(const BoundingBox& bounds,
                                      const std::string& text, int x, int y) {
    if (text.empty()) return false;
    const int bottom = std::min(bounds.bottom(), static_cast<int>(Canvas::HEIGHT) - 1);
    const int right = std::min(bounds.right(), static_cast<int>(Canvas::WIDTH) - 1);
    const int last_row = y + static_cast<int>(BitmapFont::CHARACTER_HEIGHT) - 1;
    return x >= bounds.left() && x <= right && y >= bounds.top() &&
           last_row <= bottom && text.size() <= maximumCharacters(bounds, x);
}

bool ToastNotification::fitsLine(const BoundingBox& bounds, int y) {
    const int bottom = std::min(bounds.bottom(), static_cast<int>(Canvas::HEIGHT) - 1);
    const int last_row = y + static_cast<int>(BitmapFont::CHARACTER_HEIGHT) - 1;
    return y >= bounds.top() && last_row <= bottom;
}

} // namespace ui
