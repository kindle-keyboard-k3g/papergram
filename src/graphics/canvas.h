#ifndef KINDLE_GRAPHICS_CANVAS_H
#define KINDLE_GRAPHICS_CANVAS_H

#include "../domain/value_objects.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Double-buffered 8-bit grayscale drawing surface.
 *
 * The canvas uses a fixed 600 by 800 pixel geometry. Drawing operations write
 * to the back buffer; call swapBuffers() to exchange it with the front buffer.
 */
class Canvas {
public:
    /** @brief Canvas width in pixels. */
    static constexpr std::size_t WIDTH = 600U;

    /** @brief Canvas height in pixels. */
    static constexpr std::size_t HEIGHT = 800U;

    /** @brief Number of pixels in one canvas buffer. */
    static constexpr std::size_t BUFFER_SIZE = WIDTH * HEIGHT;

    /**
     * @brief Creates a white canvas with two initialized buffers.
     */
    Canvas();

    /**
     * @brief Fills the back buffer with one grayscale color.
     *
     * @param color Color used for every pixel.
     */
    void clear(const GrayscaleColor& color);

    /**
     * @brief Sets a pixel in the back buffer.
     *
     * Coordinates outside the canvas are ignored.
     *
     * @param coordinate Pixel coordinate to update.
     * @param color New grayscale color.
     */
    void setPixel(const ScreenCoordinate& coordinate,
                  const GrayscaleColor& color);

    /**
     * @brief Sets a pixel in the back buffer.
     *
     * Coordinates outside the canvas are ignored.
     *
     * @param x Horizontal pixel coordinate.
     * @param y Vertical pixel coordinate.
     * @param color New grayscale color.
     */
    void setPixel(int x, int y, const GrayscaleColor& color);

    /**
     * @brief Reads a pixel from the back buffer.
     *
     * @param coordinate Pixel coordinate to read.
     * @return Grayscale color stored at the coordinate.
     * @throws std::out_of_range If the coordinate is outside the canvas.
     */
    GrayscaleColor pixelAt(const ScreenCoordinate& coordinate) const;

    /**
     * @brief Reads a pixel from the back buffer.
     *
     * This is an alias for pixelAt().
     *
     * @param coordinate Pixel coordinate to read.
     * @return Grayscale color stored at the coordinate.
     * @throws std::out_of_range If the coordinate is outside the canvas.
     */
    GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const;

    /**
     * @brief Draws a line into the back buffer.
     *
     * Portions outside the canvas are clipped by ignoring out-of-range pixels.
     *
     * @param start Line start coordinate.
     * @param end Line end coordinate.
     * @param color Line color.
     */
    void drawLine(const ScreenCoordinate& start,
                  const ScreenCoordinate& end,
                  const GrayscaleColor& color);

    /**
     * @brief Draws a line into the back buffer.
     *
     * Portions outside the canvas are clipped by ignoring out-of-range pixels.
     *
     * @param startX Starting horizontal coordinate.
     * @param startY Starting vertical coordinate.
     * @param endX Ending horizontal coordinate.
     * @param endY Ending vertical coordinate.
     * @param color Line color.
     */
    void drawLine(int startX, int startY, int endX, int endY,
                  const GrayscaleColor& color);

    /**
     * @brief Draws the outline of a rectangle into the back buffer.
     *
     * @param box Rectangle bounds.
     * @param color Outline color.
     */
    void drawRect(const BoundingBox& box, const GrayscaleColor& color);

    /**
     * @brief Draws the outline of a rectangle into the back buffer.
     *
     * @param left Left rectangle coordinate.
     * @param top Top rectangle coordinate.
     * @param right Right rectangle coordinate.
     * @param bottom Bottom rectangle coordinate.
     * @param color Outline color.
     */
    void drawRect(int left, int top, int right, int bottom,
                  const GrayscaleColor& color);

    /**
     * @brief Fills a rectangle in the back buffer.
     *
     * Invalid rectangles are ignored and valid portions are clipped to the
     * canvas bounds.
     *
     * @param box Rectangle bounds.
     * @param color Fill color.
     */
    void fillRect(const BoundingBox& box, const GrayscaleColor& color);

    /**
     * @brief Fills a rectangle in the back buffer.
     *
     * Invalid rectangles are ignored and valid portions are clipped to the
     * canvas bounds.
     *
     * @param left Left rectangle coordinate.
     * @param top Top rectangle coordinate.
     * @param right Right rectangle coordinate.
     * @param bottom Bottom rectangle coordinate.
     * @param color Fill color.
     */
    void fillRect(int left, int top, int right, int bottom,
                  const GrayscaleColor& color);

    /**
     * @brief Inverts all four-tone pixels in a rectangle.
     *
     * Invalid rectangles are ignored and valid portions are clipped to the
     * canvas bounds.
     *
     * @param box Rectangle bounds.
     */
    void invertRect(const BoundingBox& box);

    /**
     * @brief Inverts all four-tone pixels in a rectangle.
     *
     * Invalid rectangles are ignored and valid portions are clipped to the
     * canvas bounds.
     *
     * @param left Left rectangle coordinate.
     * @param top Top rectangle coordinate.
     * @param right Right rectangle coordinate.
     * @param bottom Bottom rectangle coordinate.
     */
    void invertRect(int left, int top, int right, int bottom);

    /**
     * @brief Renders a string into the back buffer using the embedded font.
     *
     * Characters advance by the fixed font width and out-of-range pixels are
     * ignored.
     *
     * @param origin Top-left position of the first character.
     * @param text Text to render.
     * @param color Grayscale color for the rendered glyphs.
     */
    void blitText(const ScreenCoordinate& origin, const std::string& text,
                  const GrayscaleColor& color = GrayscaleColor::BLACK);

    /**
     * @brief Renders one character into the back buffer.
     *
     * @param origin Top-left position of the character.
     * @param character Character to render.
     * @param color Grayscale color for the rendered glyph.
     */
    void blitChar(const ScreenCoordinate& origin, char character,
                  const GrayscaleColor& color = GrayscaleColor::BLACK);

    /**
     * @brief Checks whether a rectangle contains a non-white back-buffer pixel.
     *
     * The checked area is clipped to the canvas bounds.
     *
     * @param box Rectangle to inspect.
     * @return true when at least one inspected pixel is not white; otherwise false.
     */
    bool hasNonWhitePixel(const BoundingBox& box) const;

    /**
     * @brief Exchanges the front and back buffers.
     */
    void swapBuffers();

    /**
     * @brief Returns a read-only pointer to the front-buffer pixels.
     *
     * Pixels are stored in row-major order with BUFFER_SIZE elements.
     *
     * @return Non-owning pointer to the front buffer.
     */
    const std::uint8_t* frontBuffer() const;

    /**
     * @brief Returns a read-only pointer to the back-buffer pixels.
     *
     * Pixels are stored in row-major order with BUFFER_SIZE elements.
     *
     * @return Non-owning pointer to the back buffer.
     */
    const std::uint8_t* backBuffer() const;

    /**
     * @brief Writes the back buffer as a binary PPM image.
     *
     * @param filename Destination path for the PPM file.
     * @throws std::runtime_error If the file cannot be created or written.
     */
    void saveToPpm(const std::string& filename) const;

private:
    std::vector<std::uint8_t> back_buffer_;
    std::vector<std::uint8_t> front_buffer_;

    static bool isInside(int x, int y);
    static std::uint8_t inverted(std::uint8_t value);
    std::size_t indexFor(int x, int y) const;
};

#endif
