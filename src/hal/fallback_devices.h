#ifndef KINDLE_HAL_FALLBACK_DEVICES_H
#define KINDLE_HAL_FALLBACK_DEVICES_H

#include "eink_controller.h"
#include "frame_buffer.h"
#include "input_device.h"

#include <termios.h>
#include <vector>

/**
 * @brief Host-memory framebuffer used when Kindle hardware is unavailable.
 *
 * The buffer emulates the Kindle's 600 by 800 logical display and exports a
 * PPM snapshot when flushed.
 */
class MemoryFrameBuffer : public IFrameBuffer {
public:
    /** @brief Emulated display width in pixels. */
    static constexpr std::size_t WIDTH = 600U;

    /** @brief Emulated display height in pixels. */
    static constexpr std::size_t HEIGHT = 800U;

    /** @brief Creates a white emulated framebuffer. */
    MemoryFrameBuffer();

    /**
     * @brief Returns the emulated display width.
     * @return Width in pixels.
     */
    std::size_t width() const override;

    /**
     * @brief Returns the emulated display height.
     * @return Height in pixels.
     */
    std::size_t height() const override;

    /**
     * @brief Writes one pixel when the coordinate is within the display.
     * @param coordinate Pixel position to update.
     * @param color Grayscale value to store.
     */
    void setPixel(const ScreenCoordinate& coordinate, const GrayscaleColor& color) override;

    /**
     * @brief Reads one pixel, returning white for an invalid coordinate.
     * @param coordinate Pixel position to read.
     * @return Grayscale value stored at the coordinate, or white when outside
     *     the display.
     */
    GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const override;

    /**
     * @brief Fills the emulated display with one grayscale value.
     * @param color Grayscale value used for every pixel.
     */
    void clear(const GrayscaleColor& color) override;

    /**
     * @brief Copies 8-bit grayscale pixels into host memory.
     * @param buffer Source pixel bytes.
     * @param size Number of source bytes available; excess bytes are ignored.
     */
    void copyFrom(const std::uint8_t* buffer, std::size_t size) override;

    /** @brief Writes the current framebuffer to /tmp/kindle_fb.ppm. */
    void flush() override;

private:
    std::vector<GrayscaleColor> pixels_;

    void copyPixels(const std::uint8_t* buffer, std::size_t count);
};

/**
 * @brief No-op e-ink controller for host-side emulation.
 */
class DummyEinkController : public IEinkController {
public:
    /**
     * @brief Ignores a requested region refresh.
     * @param area Region that would be refreshed on Kindle hardware.
     * @param isFullRefresh Whether the request would use a full waveform.
     */
    void updateArea(const BoundingBox& area, bool isFullRefresh) override;

    /** @brief Ignores a requested full-screen refresh. */
    void fullRefresh() override;
};

/**
 * @brief Temporarily switches standard input to noncanonical mode.
 *
 * The original terminal settings are restored when the object is destroyed.
 */
class TerminalModeRestorer {
public:
    /** @brief Captures terminal settings and enables raw-style input. */
    TerminalModeRestorer();

    /** @brief Restores the captured terminal settings when active. */
    ~TerminalModeRestorer();

    /** @brief Enables noncanonical, non-echoing input when stdin is a TTY. */
    void enableRawMode();

private:
    struct termios original_settings_{};
    bool is_active_ = false;
};

/**
 * @brief Converts terminal input into normalized application key events.
 */
class StdinInputDevice : public IInputDevice {
public:
    /** @brief Creates a standard-input event source. */
    StdinInputDevice();

    /** @brief Restores terminal state through its mode restorer. */
    ~StdinInputDevice() override = default;

    /**
     * @brief Waits for and translates one character or escape sequence.
     * @param outEvent Destination for the normalized event.
     * @param timeoutMs Maximum wait time in milliseconds.
     * @return true when a supported event was read; otherwise false.
     */
    bool pollEvent(InputEvent& outEvent, int timeoutMs) override;

    /**
     * @brief Reports whether standard input has reached end-of-input.
     * @return true after EOF or a terminal input error; otherwise false.
     */
    bool isClosed() const override;

    /**
     * @brief Maps one terminal character to an application event.
     * @param ch Character to translate.
     * @param out Destination for the normalized event.
     * @return true when the character is recognized; otherwise false.
     */
    bool mapCharToEvent(char ch, InputEvent& out);

private:
    bool readChar(char& ch, int timeoutMs);
    bool mapAlphaChar(char ch, InputEvent& out);
    bool mapDigitOrControl(char ch, InputEvent& out);
    bool parseEscapeSequence(InputEvent& out);
    bool mapAnsiBracketSequence(char code, InputEvent& out);

    bool closed_ = false;
    TerminalModeRestorer terminal_mode_;
};

#endif
