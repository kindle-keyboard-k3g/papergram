#ifndef KINDLE_GRAPHICS_BITMAP_FONT_H
#define KINDLE_GRAPHICS_BITMAP_FONT_H

#include "../domain/value_objects.h"

#include <array>
#include <cstdint>

class Canvas;

class BitmapFont {
public:
    static constexpr std::size_t CHARACTER_WIDTH = 8U;
    static constexpr std::size_t CHARACTER_HEIGHT = 16U;
    using Glyph = std::array<std::uint8_t, CHARACTER_HEIGHT>;

    Glyph glyph(char character) const;
    void render(Canvas& canvas, const ScreenCoordinate& origin, char character,
                const GrayscaleColor& color) const;

private:
    static char normalized(char character);
    static std::uint8_t patternRow(char character, std::size_t row);
};

#endif
