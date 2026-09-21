#ifndef KINDLE_TEST_MOCK_FRAME_BUFFER_H
#define KINDLE_TEST_MOCK_FRAME_BUFFER_H

#include "../../src/hal/frame_buffer.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class MockFrameBuffer : public IFrameBuffer {
public:
    static constexpr std::size_t WIDTH = 600U;
    static constexpr std::size_t HEIGHT = 800U;

    MockFrameBuffer() : pixels_(WIDTH * HEIGHT, GrayscaleColor::WHITE) {}

    std::size_t width() const override {
        return WIDTH;
    }

    std::size_t height() const override {
        return HEIGHT;
    }

    void setPixel(const ScreenCoordinate& coordinate,
                  const GrayscaleColor& color) override {
        pixels_.at(indexFor(coordinate)) = color;
    }

    GrayscaleColor getPixel(const ScreenCoordinate& coordinate) const override {
        return pixels_.at(indexFor(coordinate));
    }

    void clear(const GrayscaleColor& color) override {
        std::fill(pixels_.begin(), pixels_.end(), color);
    }

    void flush() override {}

    void saveToPpm(const std::string& filename) const {
        std::ofstream output(filename, std::ios::binary);
        if (!output) {
            throw std::runtime_error("Unable to create PPM file");
        }
        output << "P6\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
        for (const GrayscaleColor& color : pixels_) {
            const char channel = static_cast<char>(color.value());
            output.write(&channel, 1);
            output.write(&channel, 1);
            output.write(&channel, 1);
        }
    }

private:
    static std::size_t indexFor(const ScreenCoordinate& coordinate) {
        if (coordinate.x() >= static_cast<int>(WIDTH) ||
            coordinate.y() >= static_cast<int>(HEIGHT)) {
            throw std::out_of_range("Pixel is outside the framebuffer");
        }
        return static_cast<std::size_t>(coordinate.y()) * WIDTH +
               static_cast<std::size_t>(coordinate.x());
    }

    std::vector<GrayscaleColor> pixels_;
};

#endif
