#ifndef KINDLE_HAL_FRAME_BUFFER_H
#define KINDLE_HAL_FRAME_BUFFER_H

#include "../domain/value_objects.h"

#include <cstddef>

class IFrameBuffer {
public:
    virtual ~IFrameBuffer() = default;

    virtual std::size_t width() const = 0;
    virtual std::size_t height() const = 0;
    virtual void setPixel(const ScreenCoordinate& coordinate,
                          const GrayscaleColor& color) = 0;
    virtual GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const = 0;
    virtual void clear(const GrayscaleColor& color) = 0;
    virtual void copyFrom(const std::uint8_t* buffer, std::size_t size) = 0;
    virtual void flush() = 0;
};

#endif
