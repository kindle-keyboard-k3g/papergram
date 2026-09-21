#include "status_header.h"
#include <cstdio>
#include <ctime>

namespace {
std::string currentClock() {
    std::time_t now = std::time(nullptr);
    struct tm ltm;
    if (localtime_r(&now, &ltm) == nullptr) return "12:00";
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", ltm.tm_hour, ltm.tm_min);
    return std::string(buf);
}
}

StatusHeader::StatusHeader(const std::string& title) {
    data_.title = title;
}

void StatusHeader::setTitle(const std::string& title) {
    data_.title = title;
}

void StatusHeader::setNetworkStatus(const std::string& status) {
    data_.network_status = status;
}

void StatusHeader::setBatteryLevel(int percentage) {
    data_.battery_text = "[" + std::to_string(percentage) + "%]";
}

void StatusHeader::render(Canvas& canvas) const {
    const BoundingBox header_box(0, 0, 600, 24);
    canvas.fillRect(header_box, GrayscaleColor::WHITE);
    canvas.blitText(ScreenCoordinate(10, 4), data_.title, GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(380, 4), data_.network_status, GrayscaleColor::DARK_GRAY);
    canvas.blitText(ScreenCoordinate(460, 4), data_.battery_text, GrayscaleColor::DARK_GRAY);
    canvas.blitText(ScreenCoordinate(530, 4), currentClock(), GrayscaleColor::BLACK);
    canvas.drawLine(ScreenCoordinate(0, 24), ScreenCoordinate(600, 24), GrayscaleColor::BLACK);
}
