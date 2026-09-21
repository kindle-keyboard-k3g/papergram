#ifndef KINDLE_UI_TOAST_NOTIFICATION_H
#define KINDLE_UI_TOAST_NOTIFICATION_H

#include "../graphics/canvas.h"
#include "../domain/value_objects.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ui {

class ToastNotification {
public:
    ToastNotification(const BoundingBox& bounds, const std::string& title,
                      const std::vector<std::string>& lines);
    void render(Canvas& canvas) const;
    const BoundingBox& bounds() const;

private:
    struct ToastData {
        BoundingBox bounds;
        std::string title;
        std::vector<std::string> lines;
    };

    ToastData data_;

    void renderBackground(Canvas& canvas) const;
    void renderBorder(Canvas& canvas) const;
    void renderTitle(Canvas& canvas) const;
    void renderLines(Canvas& canvas) const;
    void renderText(Canvas& canvas, const std::string& text, int x,
                    int y) const;
    static std::string clippedText(const BoundingBox& bounds,
                                   const std::string& text, int x);
    static std::size_t maximumCharacters(const BoundingBox& bounds, int x);
    static bool canRenderText(const BoundingBox& bounds,
                              const std::string& text, int x, int y);
    static bool fitsLine(const BoundingBox& bounds, int y);
};

} // namespace ui

#endif
