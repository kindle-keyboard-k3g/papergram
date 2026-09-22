#ifndef KINDLE_UI_TOAST_NOTIFICATION_H
#define KINDLE_UI_TOAST_NOTIFICATION_H

#include "../graphics/canvas.h"
#include "../domain/value_objects.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ui {

/**
 * @brief Renders a double-bordered notification card with text lines.
 */
class ToastNotification {
public:
    /**
     * @brief Creates a notification card.
     * @param bounds Screen rectangle occupied by the card.
     * @param title Heading rendered near the top of the card.
     * @param lines Body lines rendered below the heading.
     */
    ToastNotification(const BoundingBox& bounds, const std::string& title,
                      const std::vector<std::string>& lines);

    /**
     * @brief Moves the card to a new screen rectangle.
     * @param bounds New card rectangle.
     */
    void setBounds(const BoundingBox& bounds);

    /**
     * @brief Draws the card background, borders, title, and body text.
     * @param canvas Destination canvas.
     */
    void render(Canvas& canvas) const;

    /**
     * @brief Returns the card's current screen rectangle.
     * @return Current card bounds.
     */
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
    void renderText(Canvas& canvas, const std::string& text,
                    std::size_t maximum_characters, int x, int y) const;
    static std::size_t maximumCharacters(const BoundingBox& bounds, int x);
    static bool canRenderText(const BoundingBox& bounds,
                              std::size_t character_count, int x, int y);
    static bool fitsLine(const BoundingBox& bounds, int y);
};

} // namespace ui

#endif
