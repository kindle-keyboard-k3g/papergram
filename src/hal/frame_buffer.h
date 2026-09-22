#ifndef KINDLE_HAL_FRAME_BUFFER_H
#define KINDLE_HAL_FRAME_BUFFER_H

#include "../domain/value_objects.h"

#include <cstddef>

/**
 * @brief Abstract pixel buffer used by the rendering layer.
 *
 * Implementations may store pixels in physical video memory or in host memory
 * while exposing the same logical display operations.
 */
class IFrameBuffer {
public:
    /** @brief Releases the framebuffer resources. */
    virtual ~IFrameBuffer() = default;

    /**
     * @brief Returns the logical display width.
     * @return Width in pixels.
     */
    virtual std::size_t width() const = 0;

    /**
     * @brief Returns the logical display height.
     * @return Height in pixels.
     */
    virtual std::size_t height() const = 0;

    /**
     * @brief Writes one pixel to the buffer.
     * @param coordinate Pixel position to update.
     * @param color Grayscale value to store.
     */
    virtual void setPixel(const ScreenCoordinate& coordinate,
                          const GrayscaleColor& color) = 0;

    /**
     * @brief Reads one pixel from the buffer.
     * @param coordinate Pixel position to read.
     * @return Grayscale value stored at the coordinate.
     */
    virtual GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const = 0;

    /**
     * @brief Fills the entire buffer with one grayscale value.
     * @param color Grayscale value used for every pixel.
     */
    virtual void clear(const GrayscaleColor& color) = 0;

    /**
     * @brief Copies logical 8-bit pixels into the buffer.
     * @param buffer Source pixel bytes.
     * @param size Number of source bytes available.
     */
    virtual void copyFrom(const std::uint8_t* buffer, std::size_t size) = 0;

    /** @brief Makes buffered pixels visible on the target display. */
    virtual void flush() = 0;
};

#endif
