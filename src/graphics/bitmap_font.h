#ifndef KINDLE_GRAPHICS_BITMAP_FONT_H
#define KINDLE_GRAPHICS_BITMAP_FONT_H

#include "../domain/value_objects.h"

#include <array>
#include <cstdint>

class Canvas;

/**
 * @brief Renders the embedded fixed-width bitmap font.
 *
 * Glyphs are eight pixels wide and sixteen pixels high. Alphabetic input is
 * normalized to uppercase, while unsupported characters are rendered as '?'.
 */
class BitmapFont {
public:
    /** @brief Width of each rendered glyph in pixels. */
    static constexpr std::size_t CHARACTER_WIDTH = 8U;

    /** @brief Height of each rendered glyph in pixels. */
    static constexpr std::size_t CHARACTER_HEIGHT = 16U;

    /** @brief Pixel-row bitmap representation of one glyph. */
    using Glyph = std::array<std::uint8_t, CHARACTER_HEIGHT>;

    /**
     * @brief Returns the bitmap for one character.
     *
     * @param character Character to convert to a glyph.
     * @return A sixteen-row bitmap with one byte per row.
     */
    Glyph glyph(char character) const;

    /**
     * @brief Draws one glyph onto a canvas.
     *
     * Pixels outside the canvas are ignored by the canvas implementation.
     *
     * @param canvas Destination canvas.
     * @param origin Top-left position of the glyph.
     * @param character Character to render.
     * @param color Grayscale color used for set glyph pixels.
     */
    void render(Canvas& canvas, const ScreenCoordinate& origin, char character,
                const GrayscaleColor& color) const;

private:
    static char normalized(char character);
    static std::uint8_t patternRow(char character, std::size_t row);
};

#endif
