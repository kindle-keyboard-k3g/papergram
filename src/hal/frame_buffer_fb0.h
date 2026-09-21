#ifndef KINDLE_HAL_FRAME_BUFFER_FB0_H
#define KINDLE_HAL_FRAME_BUFFER_FB0_H

#include "frame_buffer.h"

#include <cstddef>
#include <string>

class FrameBufferLinuxFb0 final : public IFrameBuffer {
public:
    static constexpr std::size_t WIDTH = 600U;
    static constexpr std::size_t HEIGHT = 800U;
    static constexpr std::size_t BUFFER_SIZE = (WIDTH * HEIGHT) / 2U;

    explicit FrameBufferLinuxFb0(const std::string& devicePath = "/dev/fb0");
    ~FrameBufferLinuxFb0() override;

    FrameBufferLinuxFb0(const FrameBufferLinuxFb0&) = delete;
    FrameBufferLinuxFb0& operator=(const FrameBufferLinuxFb0&) = delete;

    std::size_t width() const override;
    std::size_t height() const override;
    void setPixel(const ScreenCoordinate& coordinate,
                  const GrayscaleColor& color) override;
    GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const override;
    void clear(const GrayscaleColor& color) override;
    void copyFrom(const std::uint8_t* buffer, std::size_t size) override;
    void flush() override;
    int fileDescriptor() const;

private:
    int file_descriptor_;
    unsigned char* mapped_memory_;

    void pack8bppTo4bpp(const std::uint8_t* buffer, std::size_t count);
    void setEvenPixel(std::size_t byte_index, std::uint8_t nibble, int x);
    void setOddPixel(std::size_t byte_index, std::uint8_t nibble, int x);
    static std::size_t indexFor(const ScreenCoordinate& coordinate);
};

#endif
