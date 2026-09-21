#include "popup_types.h"

#include <stdexcept>

namespace {

const std::chrono::milliseconds DEFAULT_POPUP_DURATION(4000);

void requirePositiveDuration(const std::chrono::milliseconds& value) {
    if (value.count() <= 0) {
        throw std::invalid_argument("Popup duration must be positive");
    }
}

} // namespace

namespace ui {

PopupDuration::PopupDuration() : PopupDuration(DEFAULT_POPUP_DURATION) {}

PopupDuration::PopupDuration(std::chrono::milliseconds value) : value_(value) {
    requirePositiveDuration(value_);
}

std::chrono::milliseconds PopupDuration::toMillis() const {
    return value_;
}

PopupDuration PopupDuration::defaultValue() {
    return PopupDuration(DEFAULT_POPUP_DURATION);
}

PopupTimestamp::PopupTimestamp(std::chrono::milliseconds value) : value_(value) {}

std::chrono::milliseconds PopupTimestamp::millis() const {
    return value_;
}

PopupTimestamp PopupTimestamp::operator+(std::chrono::milliseconds duration) const {
    return PopupTimestamp(value_ + duration);
}

PopupTimestamp PopupTimestamp::operator-(std::chrono::milliseconds duration) const {
    return PopupTimestamp(value_ - duration);
}

std::chrono::milliseconds PopupTimestamp::operator-(const PopupTimestamp& other) const {
    return value_ - other.value_;
}

bool PopupTimestamp::operator>=(const PopupTimestamp& other) const {
    return value_ >= other.value_;
}

bool PopupTimestamp::operator<(const PopupTimestamp& other) const {
    return value_ < other.value_;
}

} // namespace ui
