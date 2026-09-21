#include "fallback_devices.h"
#include <algorithm>
#include <fstream>
#include <poll.h>
#include <unistd.h>

MemoryFrameBuffer::MemoryFrameBuffer()
    : pixels_(WIDTH * HEIGHT, GrayscaleColor::WHITE) {}

std::size_t MemoryFrameBuffer::width() const { return WIDTH; }
std::size_t MemoryFrameBuffer::height() const { return HEIGHT; }

void MemoryFrameBuffer::setPixel(const ScreenCoordinate& coordinate, const GrayscaleColor& color) {
    if (coordinate.x() < 0 || coordinate.x() >= static_cast<int>(WIDTH)) return;
    if (coordinate.y() < 0 || coordinate.y() >= static_cast<int>(HEIGHT)) return;
    std::size_t idx = static_cast<std::size_t>(coordinate.y()) * WIDTH + coordinate.x();
    pixels_[idx] = color;
}

GrayscaleColor MemoryFrameBuffer::getPixel(const ScreenCoordinate& coordinate) const {
    if (coordinate.x() < 0 || coordinate.x() >= static_cast<int>(WIDTH)) return GrayscaleColor::WHITE;
    if (coordinate.y() < 0 || coordinate.y() >= static_cast<int>(HEIGHT)) return GrayscaleColor::WHITE;
    std::size_t idx = static_cast<std::size_t>(coordinate.y()) * WIDTH + coordinate.x();
    return pixels_[idx];
}

void MemoryFrameBuffer::clear(const GrayscaleColor& color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void MemoryFrameBuffer::flush() {
    std::ofstream out("/tmp/kindle_fb.ppm", std::ios::binary);
    if (!out) return;
    out << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    for (const auto& color : pixels_) {
        char ch = static_cast<char>(color.value());
        out.write(&ch, 1);
        out.write(&ch, 1);
        out.write(&ch, 1);
    }
}

void DummyEinkController::updateArea(const BoundingBox& area, bool isFullRefresh) {
    static_cast<void>(area);
    static_cast<void>(isFullRefresh);
}

void DummyEinkController::fullRefresh() {}

bool StdinInputDevice::pollEvent(InputEvent& outEvent, int timeoutMs) {
    char ch = '\0';
    if (!readChar(ch, timeoutMs)) return false;
    return mapCharToEvent(ch, outEvent);
}

bool StdinInputDevice::readChar(char& ch, int timeoutMs) {
    struct pollfd pfd{STDIN_FILENO, POLLIN, 0};
    if (poll(&pfd, 1, timeoutMs) <= 0) return false;
    return read(STDIN_FILENO, &ch, 1) == 1;
}

bool StdinInputDevice::mapCharToEvent(char ch, InputEvent& out) {
    out.pressed = true;
    if (ch >= 'a' && ch <= 'z') {
        out.code = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + (ch - 'a'));
        return true;
    }
    if (ch >= 'A' && ch <= 'Z') {
        out.code = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + (ch - 'A'));
        return true;
    }
    if (ch >= '1' && ch <= '9') {
        out.code = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_1) + (ch - '1'));
        return true;
    }
    if (ch == '0') { out.code = KeyCode::KEY_0; return true; }
    if (ch == '\n' || ch == '\r') { out.code = KeyCode::KEY_ENTER; return true; }
    if (ch == 127 || ch == '\b') { out.code = KeyCode::KEY_BACKSPACE; return true; }
    if (ch == ' ') { out.code = KeyCode::KEY_SPACE; return true; }
    if (ch == 27) { out.code = KeyCode::KEY_BACK; return true; }
    return false;
}
