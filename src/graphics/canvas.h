#ifndef KINDLE_GRAPHICS_CANVAS_H
#define KINDLE_GRAPHICS_CANVAS_H

#include "../domain/value_objects.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class Canvas {
public:
    static constexpr std::size_t WIDTH = 600U;
    static constexpr std::size_t HEIGHT = 800U;
    static constexpr std::size_t BUFFER_SIZE = WIDTH * HEIGHT;

    Canvas();

    void clear(const GrayscaleColor& color);
    void setPixel(const ScreenCoordinate& coordinate,
                  const GrayscaleColor& color);
    void setPixel(int x, int y, const GrayscaleColor& color);
    GrayscaleColor pixelAt(const ScreenCoordinate& coordinate) const;
    GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const;

    void drawLine(const ScreenCoordinate& start,
                  const ScreenCoordinate& end,
                  const GrayscaleColor& color);
    void drawLine(int startX, int startY, int endX, int endY,
                  const GrayscaleColor& color);
    void drawRect(const BoundingBox& box, const GrayscaleColor& color);
    void drawRect(int left, int top, int right, int bottom,
                  const GrayscaleColor& color);
    void fillRect(const BoundingBox& box, const GrayscaleColor& color);
    void fillRect(int left, int top, int right, int bottom,
                  const GrayscaleColor& color);
    void invertRect(const BoundingBox& box);
    void invertRect(int left, int top, int right, int bottom);

    void blitText(const ScreenCoordinate& origin, const std::string& text,
                  const GrayscaleColor& color = GrayscaleColor::BLACK);
    void blitChar(const ScreenCoordinate& origin, char character,
                  const GrayscaleColor& color = GrayscaleColor::BLACK);

    bool hasNonWhitePixel(const BoundingBox& box) const;
    void swapBuffers();
    const std::uint8_t* frontBuffer() const;
    const std::uint8_t* backBuffer() const;
    void saveToPpm(const std::string& filename) const;

private:
    std::vector<std::uint8_t> back_buffer_;
    std::vector<std::uint8_t> front_buffer_;

    static bool isInside(int x, int y);
    static std::uint8_t inverted(std::uint8_t value);
    std::size_t indexFor(int x, int y) const;
};

#endif
