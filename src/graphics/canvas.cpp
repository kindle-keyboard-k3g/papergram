#include "canvas.h"

#include "bitmap_font.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

Canvas::Canvas()
    : back_buffer_(BUFFER_SIZE, GrayscaleColor::WHITE.value()),
      front_buffer_(BUFFER_SIZE, GrayscaleColor::WHITE.value()) {}

void Canvas::clear(const GrayscaleColor& color) {
    std::fill(back_buffer_.begin(), back_buffer_.end(), color.value());
}

void Canvas::setPixel(const ScreenCoordinate& coordinate,
                      const GrayscaleColor& color) {
    setPixel(coordinate.x(), coordinate.y(), color);
}

void Canvas::setPixel(int x, int y, const GrayscaleColor& color) {
    if (!isInside(x, y)) {
        return;
    }
    back_buffer_.at(indexFor(x, y)) = color.value();
}

GrayscaleColor Canvas::pixelAt(const ScreenCoordinate& coordinate) const {
    if (!isInside(coordinate.x(), coordinate.y())) {
        throw std::out_of_range("Pixel is outside the canvas");
    }
    return GrayscaleColor(back_buffer_.at(indexFor(coordinate.x(), coordinate.y())));
}

GrayscaleColor Canvas::getPixel(const ScreenCoordinate& coordinate) const {
    return pixelAt(coordinate);
}

void Canvas::drawLine(const ScreenCoordinate& start,
                      const ScreenCoordinate& end,
                      const GrayscaleColor& color) {
    drawLine(start.x(), start.y(), end.x(), end.y(), color);
}

void Canvas::drawLine(int startX, int startY, int endX, int endY,
                      const GrayscaleColor& color) {
    int x = startX;
    int y = startY;
    const int delta_x = std::abs(endX - x);
    const int step_x = x < endX ? 1 : -1;
    const int delta_y = -std::abs(endY - y);
    const int step_y = y < endY ? 1 : -1;
    int error = delta_x + delta_y;
    while (true) {
        setPixel(x, y, color);
        if (x == endX && y == endY) {
            return;
        }
        const int doubled_error = 2 * error;
        if (doubled_error >= delta_y) {
            error += delta_y;
            x += step_x;
        }
        if (doubled_error <= delta_x) {
            error += delta_x;
            y += step_y;
        }
    }
}

void Canvas::drawRect(const BoundingBox& box, const GrayscaleColor& color) {
    drawRect(box.left(), box.top(), box.right(), box.bottom(), color);
}

void Canvas::drawRect(int left, int top, int right, int bottom,
                      const GrayscaleColor& color) {
    drawLine(left, top, right, top, color);
    drawLine(right, top, right, bottom, color);
    drawLine(right, bottom, left, bottom, color);
    drawLine(left, bottom, left, top, color);
}

void Canvas::fillRect(const BoundingBox& box, const GrayscaleColor& color) {
    fillRect(box.left(), box.top(), box.right(), box.bottom(), color);
}

void Canvas::fillRect(int left, int top, int right, int bottom,
                      const GrayscaleColor& color) {
    if (left > right || top > bottom) {
        return;
    }
    const int first_x = std::max(left, 0);
    const int last_x = std::min(right, static_cast<int>(WIDTH) - 1);
    const int first_y = std::max(top, 0);
    const int last_y = std::min(bottom, static_cast<int>(HEIGHT) - 1);
    for (int y = first_y; y <= last_y; ++y) {
        for (int x = first_x; x <= last_x; ++x) {
            setPixel(x, y, color);
        }
    }
}

void Canvas::invertRect(const BoundingBox& box) {
    invertRect(box.left(), box.top(), box.right(), box.bottom());
}

void Canvas::invertRect(int left, int top, int right, int bottom) {
    if (left > right || top > bottom) {
        return;
    }
    const int first_x = std::max(left, 0);
    const int last_x = std::min(right, static_cast<int>(WIDTH) - 1);
    const int first_y = std::max(top, 0);
    const int last_y = std::min(bottom, static_cast<int>(HEIGHT) - 1);
    for (int y = first_y; y <= last_y; ++y) {
        for (int x = first_x; x <= last_x; ++x) {
            const std::size_t index = indexFor(x, y);
            back_buffer_.at(index) = inverted(back_buffer_.at(index));
        }
    }
}

void Canvas::blitText(const ScreenCoordinate& origin, const std::string& text,
                      const GrayscaleColor& color) {
    int x = origin.x();
    for (const char character : text) {
        blitChar(ScreenCoordinate(std::min(x, 600), origin.y()), character, color);
        x += static_cast<int>(BitmapFont::CHARACTER_WIDTH);
    }
}

void Canvas::blitChar(const ScreenCoordinate& origin, char character,
                      const GrayscaleColor& color) {
    BitmapFont().render(*this, origin, character, color);
}

bool Canvas::hasNonWhitePixel(const BoundingBox& box) const {
    const int first_x = std::max(box.left(), 0);
    const int last_x = std::min(box.right(), static_cast<int>(WIDTH) - 1);
    const int first_y = std::max(box.top(), 0);
    const int last_y = std::min(box.bottom(), static_cast<int>(HEIGHT) - 1);
    for (int y = first_y; y <= last_y; ++y) {
        for (int x = first_x; x <= last_x; ++x) {
            if (back_buffer_.at(indexFor(x, y)) != GrayscaleColor::WHITE.value()) {
                return true;
            }
        }
    }
    return false;
}

void Canvas::swapBuffers() {
    back_buffer_.swap(front_buffer_);
}

const std::uint8_t* Canvas::frontBuffer() const {
    return front_buffer_.data();
}

const std::uint8_t* Canvas::backBuffer() const {
    return back_buffer_.data();
}

void Canvas::saveToPpm(const std::string& filename) const {
    std::ofstream output(filename, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Unable to create PPM file");
    }
    output << "P6\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
    for (const std::uint8_t value : back_buffer_) {
        const char channel = static_cast<char>(value);
        output.write(&channel, 1);
        output.write(&channel, 1);
        output.write(&channel, 1);
    }
    if (!output) {
        throw std::runtime_error("Unable to write PPM file");
    }
}

bool Canvas::isInside(int x, int y) {
    return x >= 0 && x < static_cast<int>(WIDTH) && y >= 0 &&
           y < static_cast<int>(HEIGHT);
}

std::uint8_t Canvas::inverted(std::uint8_t value) {
    if (value == GrayscaleColor::BLACK.value()) {
        return GrayscaleColor::WHITE.value();
    }
    if (value == GrayscaleColor::WHITE.value()) {
        return GrayscaleColor::BLACK.value();
    }
    if (value == GrayscaleColor::DARK_GRAY.value()) {
        return GrayscaleColor::LIGHT_GRAY.value();
    }
    return GrayscaleColor::DARK_GRAY.value();
}

std::size_t Canvas::indexFor(int x, int y) const {
    return static_cast<std::size_t>(y) * WIDTH + static_cast<std::size_t>(x);
}
