#ifndef KINDLE_HAL_FRAME_BUFFER_FB0_H
#define KINDLE_HAL_FRAME_BUFFER_FB0_H

#include "frame_buffer.h"

#include <cstddef>
#include <string>

/**
 * @brief Memory-mapped Linux framebuffer implementation for /dev/fb0.
 *
 * The logical display is exposed as an 8-bit grayscale buffer and packed into
 * the Kindle's 4-bit framebuffer representation when copied to video memory.
 */
class FrameBufferLinuxFb0 final : public IFrameBuffer {
public:
    /** @brief Logical display width in pixels. */
    static constexpr std::size_t WIDTH = 600U;

    /** @brief Logical display height in pixels. */
    static constexpr std::size_t HEIGHT = 800U;

    /** @brief Physical framebuffer mapping size in bytes. */
    static constexpr std::size_t BUFFER_SIZE = (WIDTH * HEIGHT) / 2U;

    /**
     * @brief Opens and maps a Linux framebuffer device.
     * @param devicePath Path to the framebuffer device.
     * @throws std::system_error If the device cannot be opened or mapped.
     */
    explicit FrameBufferLinuxFb0(const std::string& devicePath = "/dev/fb0");

    /** @brief Unmaps the framebuffer and closes its file descriptor. */
    ~FrameBufferLinuxFb0() override;

    /** @brief Disables copying because the framebuffer mapping is unique. */
    FrameBufferLinuxFb0(const FrameBufferLinuxFb0&) = delete;

    /** @brief Disables assignment because the framebuffer mapping is unique. */
    FrameBufferLinuxFb0& operator=(const FrameBufferLinuxFb0&) = delete;

    /**
     * @brief Returns the logical display width.
     * @return Width in pixels.
     */
    std::size_t width() const override;

    /**
     * @brief Returns the logical display height.
     * @return Height in pixels.
     */
    std::size_t height() const override;

    /**
     * @brief Writes a grayscale pixel into mapped framebuffer memory.
     * @param coordinate Pixel position to update.
     * @param color Grayscale value to store.
     * @throws std::out_of_range If the coordinate is outside the display.
     */
    void setPixel(const ScreenCoordinate& coordinate,
                  const GrayscaleColor& color) override;

    /**
     * @brief Reads a grayscale pixel from mapped framebuffer memory.
     * @param coordinate Pixel position to read.
     * @return Grayscale value stored at the coordinate.
     * @throws std::out_of_range If the coordinate is outside the display.
     */
    GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const override;

    /**
     * @brief Fills the mapped framebuffer with one grayscale value.
     * @param color Grayscale value used for every pixel.
     */
    void clear(const GrayscaleColor& color) override;

    /**
     * @brief Packs logical 8-bit pixels into the mapped 4-bit framebuffer.
     * @param buffer Source pixel bytes.
     * @param size Number of source bytes available; excess bytes are ignored.
     */
    void copyFrom(const std::uint8_t* buffer, std::size_t size) override;

    /**
     * @brief Flushes the mapped framebuffer.
     *
     * The mapping is shared with video memory, so display refresh itself is
     * requested separately through the e-ink controller.
     */
    void flush() override;

    /**
     * @brief Returns the underlying framebuffer file descriptor.
     * @return The open framebuffer file descriptor.
     */
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
