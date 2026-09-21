#include "toast_notification.h"

namespace ui {

ToastNotification::ToastNotification(const BoundingBox& bounds,
                                     const std::string& title,
                                     const std::vector<std::string>& lines)
    : data_{bounds, title, lines} {}

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
    const ScreenCoordinate origin(data_.bounds.left() + 12,
                                  data_.bounds.top() + 8);
    canvas.blitText(origin, data_.title, GrayscaleColor::BLACK);
}

void ToastNotification::renderLines(Canvas& canvas) const {
    int y = data_.bounds.top() + 30;
    for (std::size_t index = 0U;
         index < data_.lines.size() && fitsLine(data_.bounds, y); ++index) {
        canvas.blitText(ScreenCoordinate(data_.bounds.left() + 12, y),
                        data_.lines[index], GrayscaleColor::BLACK);
        y += 18;
    }
}

bool ToastNotification::fitsLine(const BoundingBox& bounds, int y) {
    return y >= bounds.top() && y + 15 <= bounds.bottom();
}

} // namespace ui
