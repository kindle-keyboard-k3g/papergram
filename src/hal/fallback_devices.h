#ifndef KINDLE_HAL_FALLBACK_DEVICES_H
#define KINDLE_HAL_FALLBACK_DEVICES_H

#include "eink_controller.h"
#include "frame_buffer.h"
#include "input_device.h"

#include <termios.h>
#include <vector>

class MemoryFrameBuffer : public IFrameBuffer {
public:
    static constexpr std::size_t WIDTH = 600U;
    static constexpr std::size_t HEIGHT = 800U;

    MemoryFrameBuffer();

    std::size_t width() const override;
    std::size_t height() const override;
    void setPixel(const ScreenCoordinate& coordinate, const GrayscaleColor& color) override;
    GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const override;
    void clear(const GrayscaleColor& color) override;
    void copyFrom(const std::uint8_t* buffer, std::size_t size) override;
    void flush() override;

private:
    std::vector<GrayscaleColor> pixels_;

    void copyPixels(const std::uint8_t* buffer, std::size_t count);
};

class DummyEinkController : public IEinkController {
public:
    void updateArea(const BoundingBox& area, bool isFullRefresh) override;
    void fullRefresh() override;
};

class TerminalModeRestorer {
public:
    TerminalModeRestorer();
    ~TerminalModeRestorer();
    void enableRawMode();

private:
    struct termios original_settings_{};
    bool is_active_ = false;
};

class StdinInputDevice : public IInputDevice {
public:
    StdinInputDevice();
    ~StdinInputDevice() override = default;

    bool pollEvent(InputEvent& outEvent, int timeoutMs) override;
    bool isClosed() const override;
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
