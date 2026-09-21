#include "fallback_devices.h"
#include "util/debug_log.h"
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

void MemoryFrameBuffer::copyFrom(const std::uint8_t* buffer, std::size_t size) {
    if (buffer == nullptr) return;
    copyPixels(buffer, std::min(size, pixels_.size()));
}

void MemoryFrameBuffer::copyPixels(const std::uint8_t* buffer, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        pixels_[i] = GrayscaleColor(buffer[i]);
    }
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

TerminalModeRestorer::TerminalModeRestorer() {
    enableRawMode();
}

TerminalModeRestorer::~TerminalModeRestorer() {
    if (!is_active_) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &original_settings_);
}

void TerminalModeRestorer::enableRawMode() {
    if (!isatty(STDIN_FILENO)) return;
    if (tcgetattr(STDIN_FILENO, &original_settings_) != 0) return;
    struct termios raw = original_settings_;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
        is_active_ = true;
    }
}

StdinInputDevice::StdinInputDevice() = default;

bool StdinInputDevice::isClosed() const {
    return closed_;
}

bool StdinInputDevice::pollEvent(InputEvent& outEvent, int timeoutMs) {
    if (closed_) return false;
    char ch = '\0';
    if (!readChar(ch, timeoutMs)) return false;
    return mapCharToEvent(ch, outEvent);
}

bool StdinInputDevice::readChar(char& ch, int timeoutMs) {
    if (closed_) return false;
    struct pollfd pfd{STDIN_FILENO, POLLIN, 0};
    int ret = poll(&pfd, 1, timeoutMs);
    if (ret <= 0) return false;
    if (pfd.revents & POLLIN) {
        ssize_t bytes = read(STDIN_FILENO, &ch, 1);
        if (bytes == 1) return true;
    }
    if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
        closed_ = true;
    }
    return false;
}

bool StdinInputDevice::parseEscapeSequence(InputEvent& out) {
    char seq[2] = {0, 0};
    if (!readChar(seq[0], 20)) {
        out.code = KeyCode::KEY_BACK;
        DEBUG_LOG("Input", "Mapped standalone escape to KEY_BACK");
        return true;
    }
    if (seq[0] != '[') return false;
    if (!readChar(seq[1], 20)) return false;
    return mapAnsiBracketSequence(seq[1], out);
}

bool StdinInputDevice::mapAnsiBracketSequence(char code, InputEvent& out) {
    if (code == 'A') { out.code = KeyCode::KEY_UP; DEBUG_LOG("Input", "Mapped ANSI sequence to KEY_UP"); return true; }
    if (code == 'B') { out.code = KeyCode::KEY_DOWN; DEBUG_LOG("Input", "Mapped ANSI sequence to KEY_DOWN"); return true; }
    if (code == 'C') { out.code = KeyCode::KEY_RIGHT; DEBUG_LOG("Input", "Mapped ANSI sequence to KEY_RIGHT"); return true; }
    if (code == 'D') { out.code = KeyCode::KEY_LEFT; DEBUG_LOG("Input", "Mapped ANSI sequence to KEY_LEFT"); return true; }
    if (code == '5') { out.code = KeyCode::KEY_PAGEUP; DEBUG_LOG("Input", "Mapped ANSI sequence to KEY_PAGEUP"); return true; }
    if (code == '6') { out.code = KeyCode::KEY_PAGEDOWN; DEBUG_LOG("Input", "Mapped ANSI sequence to KEY_PAGEDOWN"); return true; }
    return false;
}

bool StdinInputDevice::mapAlphaChar(char ch, InputEvent& out) {
    if (ch >= 'a' && ch <= 'z') {
        out.code = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + (ch - 'a'));
        DEBUG_LOG("Input", std::string("Mapped char '") + ch + "' to KeyCode");
        return true;
    }
    if (ch >= 'A' && ch <= 'Z') {
        out.code = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + (ch - 'A'));
        DEBUG_LOG("Input", std::string("Mapped char '") + ch + "' to KeyCode");
        return true;
    }
    return false;
}

bool StdinInputDevice::mapDigitOrControl(char ch, InputEvent& out) {
    if (ch >= '1' && ch <= '9') {
        out.code = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_1) + (ch - '1'));
        DEBUG_LOG("Input", std::string("Mapped digit '") + ch + "' to KeyCode");
        return true;
    }
    if (ch == '0') { out.code = KeyCode::KEY_0; DEBUG_LOG("Input", "Mapped '0' to KEY_0"); return true; }
    if (ch == '\n' || ch == '\r') { out.code = KeyCode::KEY_ENTER; DEBUG_LOG("Input", "Mapped newline to KEY_ENTER"); return true; }
    if (ch == 127 || ch == '\b') { out.code = KeyCode::KEY_BACKSPACE; DEBUG_LOG("Input", "Mapped backspace to KEY_BACKSPACE"); return true; }
    if (ch == ' ') { out.code = KeyCode::KEY_SPACE; DEBUG_LOG("Input", "Mapped space to KEY_SPACE"); return true; }
    return false;
}

bool StdinInputDevice::mapCharToEvent(char ch, InputEvent& out) {
    out.pressed = true;
    if (ch == 27) return parseEscapeSequence(out);
    if (ch == 4) { closed_ = true; return false; }
    if (ch == '~') { out.code = KeyCode::KEY_POWER; return true; }
    if (ch == 'm' || ch == 'M') { out.code = KeyCode::KEY_MENU; return true; }
    if (mapAlphaChar(ch, out)) return true;
    return mapDigitOrControl(ch, out);
}

