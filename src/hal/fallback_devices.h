#ifndef KINDLE_HAL_FALLBACK_DEVICES_H
#define KINDLE_HAL_FALLBACK_DEVICES_H

#include "eink_controller.h"
#include "frame_buffer.h"
#include "input_device.h"

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
    void flush() override;

private:
    std::vector<GrayscaleColor> pixels_;
};

class DummyEinkController : public IEinkController {
public:
    void updateArea(const BoundingBox& area, bool isFullRefresh) override;
    void fullRefresh() override;
};

class StdinInputDevice : public IInputDevice {
public:
    bool pollEvent(InputEvent& outEvent, int timeoutMs) override;

private:
    bool readChar(char& ch, int timeoutMs);
    bool mapCharToEvent(char ch, InputEvent& out);
};

#endif
