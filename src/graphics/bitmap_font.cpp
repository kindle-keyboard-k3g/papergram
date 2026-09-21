#include "bitmap_font.h"

#include "canvas.h"

#include <cctype>

char BitmapFont::normalized(char character) {
    const unsigned char value = static_cast<unsigned char>(character);
    if (value >= 'a' && value <= 'z') {
        return static_cast<char>(value - ('a' - 'A'));
    }
    if (value < 32U || value > 126U) {
        return '?';
    }
    return static_cast<char>(value);
}

std::uint8_t BitmapFont::patternRow(char character, std::size_t row) {
    static const std::uint8_t letters[26][7] = {
        {14, 17, 17, 31, 17, 17, 17}, {30, 17, 17, 30, 17, 17, 30},
        {14, 17, 16, 16, 16, 17, 14}, {30, 17, 17, 17, 17, 17, 30},
        {31, 16, 16, 30, 16, 16, 31}, {31, 16, 16, 30, 16, 16, 16},
        {14, 17, 16, 23, 17, 17, 14}, {17, 17, 17, 31, 17, 17, 17},
        {14, 4, 4, 4, 4, 4, 14}, {7, 2, 2, 2, 18, 18, 12},
        {17, 18, 20, 24, 20, 18, 17}, {16, 16, 16, 16, 16, 16, 31},
        {17, 27, 21, 21, 17, 17, 17}, {17, 25, 21, 19, 17, 17, 17},
        {14, 17, 17, 17, 17, 17, 14}, {30, 17, 17, 30, 16, 16, 16},
        {14, 17, 17, 17, 21, 18, 13}, {30, 17, 17, 30, 20, 18, 17},
        {15, 16, 16, 14, 1, 1, 30}, {31, 4, 4, 4, 4, 4, 4},
        {17, 17, 17, 17, 17, 17, 14}, {17, 17, 17, 17, 17, 10, 4},
        {17, 17, 17, 21, 21, 21, 10}, {17, 17, 10, 4, 10, 17, 17},
        {17, 17, 10, 4, 4, 4, 4}, {31, 1, 2, 4, 8, 16, 31}};
    static const std::uint8_t digits[10][7] = {
        {14, 17, 19, 21, 25, 17, 14}, {4, 12, 4, 4, 4, 4, 14},
        {14, 17, 1, 2, 4, 8, 31}, {30, 1, 1, 14, 1, 1, 30},
        {2, 6, 10, 18, 31, 2, 2}, {31, 16, 16, 30, 1, 1, 30},
        {14, 16, 16, 30, 17, 17, 14}, {31, 1, 2, 4, 8, 8, 8},
        {14, 17, 17, 14, 17, 17, 14}, {14, 17, 17, 15, 1, 1, 14}};
    const char upper = normalized(character);
    if (upper >= 'A' && upper <= 'Z') {
        return letters[upper - 'A'][row];
    }
    if (upper >= '0' && upper <= '9') {
        return digits[upper - '0'][row];
    }
    if (upper == ' ') {
        return 0U;
    }
    switch (upper) {
        case '.': return row == 6U ? 4U : 0U;
        case ',': return row == 6U ? 4U : (row == 5U ? 8U : 0U);
        case '-': return row == 3U ? 14U : 0U;
        case '_': return row == 6U ? 31U : 0U;
        case ':': return row == 2U || row == 5U ? 4U : 0U;
        case '!': return row == 6U ? 4U : (row < 5U ? 4U : 0U);
        case '?': return row == 0U ? 14U : (row == 1U ? 17U :
                         (row == 2U ? 1U : (row == 3U ? 2U :
                         (row == 4U ? 4U : (row == 6U ? 4U : 0U)))));
        default: return static_cast<std::uint8_t>((character + row * 3U) & 0x1FU);
    }
}

BitmapFont::Glyph BitmapFont::glyph(char character) const {
    Glyph result{};
    for (std::size_t row = 1U; row < CHARACTER_HEIGHT - 1U; ++row) {
        const std::size_t source_row = (row - 1U) / 2U;
        result[row] = static_cast<std::uint8_t>(patternRow(character, source_row) << 1U);
    }
    return result;
}

void BitmapFont::render(Canvas& canvas, const ScreenCoordinate& origin,
                        char character, const GrayscaleColor& color) const {
    const Glyph bitmap = glyph(character);
    for (std::size_t row = 0U; row < CHARACTER_HEIGHT; ++row) {
        for (std::size_t column = 0U; column < CHARACTER_WIDTH; ++column) {
            const std::uint8_t mask = static_cast<std::uint8_t>(1U <<
                (CHARACTER_WIDTH - column - 1U));
            if ((bitmap[row] & mask) != 0U) {
                canvas.setPixel(origin.x() + static_cast<int>(column),
                                origin.y() + static_cast<int>(row), color);
            }
        }
    }
}
