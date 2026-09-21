#include "value_objects.h"

#include <cstddef>
#include <stdexcept>

namespace {

void requirePhoneNumber(const std::string& value) {
    if (value.empty() || value.front() != '+' || value.size() > 16U) {
        throw std::invalid_argument("Phone number must use E.164 format");
    }
    if (value.size() == 1U || value[1] == '0') {
        throw std::invalid_argument("Phone number must start with a non-zero digit");
    }
    for (std::size_t index = 1U; index < value.size(); ++index) {
        if (value[index] < '0' || value[index] > '9') {
            throw std::invalid_argument("Phone number contains an invalid character");
        }
    }
}

void requireAuthCode(const std::string& value) {
    if (value.size() != 5U) {
        throw std::invalid_argument("Authentication code must contain five digits");
    }
    for (const char character : value) {
        if (character < '0' || character > '9') {
            throw std::invalid_argument("Authentication code must contain only digits");
        }
    }
}

void requireCloudPassword(const std::string& value) {
    if (value.empty() || value.size() > 256U) {
        throw std::invalid_argument("Cloud password length is out of range");
    }
}

bool isContinuationByte(unsigned char byte) {
    return byte >= 0x80U && byte <= 0xBFU;
}

std::size_t utf8SequenceLength(unsigned char first) {
    if (first <= 0x7FU) {
        return 1U;
    }
    if (first >= 0xC2U && first <= 0xDFU) {
        return 2U;
    }
    if (first >= 0xE0U && first <= 0xEFU) {
        return 3U;
    }
    if (first >= 0xF0U && first <= 0xF4U) {
        return 4U;
    }
    return 0U;
}

bool hasContinuationBytes(const std::string& value,
                          std::size_t index,
                          std::size_t length) {
    for (std::size_t position = 1U; position < length; ++position) {
        if (index + position >= value.size() ||
            !isContinuationByte(static_cast<unsigned char>(value[index + position]))) {
            return false;
        }
    }
    return true;
}

bool hasValidLeadingPair(unsigned char first, unsigned char second) {
    if (first == 0xE0U && second < 0xA0U) {
        return false;
    }
    if (first == 0xEDU && second >= 0xA0U) {
        return false;
    }
    if (first == 0xF0U && second < 0x90U) {
        return false;
    }
    return first != 0xF4U || second <= 0x8FU;
}

bool isValidUtf8(const std::string& value) {
    std::size_t index = 0U;
    while (index < value.size()) {
        const unsigned char first = static_cast<unsigned char>(value[index]);
        const std::size_t length = utf8SequenceLength(first);
        if (length == 0U || !hasContinuationBytes(value, index, length)) {
            return false;
        }
        if (length > 1U) {
            const unsigned char second = static_cast<unsigned char>(value[index + 1U]);
            if (!hasValidLeadingPair(first, second)) {
                return false;
            }
        }
        index += length;
    }
    return true;
}

void requireMessageText(const std::string& value) {
    if (value.empty() || value.size() > 4096U || !isValidUtf8(value)) {
        throw std::invalid_argument("Message text must be valid UTF-8 and contain 1-4096 bytes");
    }
}

void requireChatId(std::int64_t value) {
    if (value == 0) {
        throw std::invalid_argument("Chat ID cannot be zero");
    }
}

void requireMessageId(std::int32_t value) {
    if (value <= 0) {
        throw std::invalid_argument("Message ID must be positive");
    }
}

void requireCoordinate(int x, int y) {
    if (x < 0 || x > 600 || y < 0 || y > 800) {
        throw std::out_of_range("Screen coordinate is outside the Kindle display");
    }
}

void requireBounds(const ScreenCoordinate& top_left,
                   const ScreenCoordinate& bottom_right) {
    if (top_left.x() > bottom_right.x() || top_left.y() > bottom_right.y()) {
        throw std::invalid_argument("Bounding box corners are out of order");
    }
}

bool isSupportedColor(std::uint8_t value) {
    return value == 0x00U || value == 0x55U || value == 0xAAU || value == 0xFFU;
}

}  // namespace

PhoneNumber::PhoneNumber(const std::string& value) : value_(value) {
    requirePhoneNumber(value_);
}

const std::string& PhoneNumber::value() const {
    return value_;
}

bool PhoneNumber::operator==(const PhoneNumber& other) const {
    return value_ == other.value_;
}

AuthCode::AuthCode(const std::string& value) : value_(value) {
    requireAuthCode(value_);
}

const std::string& AuthCode::value() const {
    return value_;
}

bool AuthCode::operator==(const AuthCode& other) const {
    return value_ == other.value_;
}

CloudPassword::CloudPassword(const std::string& value) : value_(value) {
    requireCloudPassword(value_);
}

const std::string& CloudPassword::value() const {
    return value_;
}

bool CloudPassword::operator==(const CloudPassword& other) const {
    return value_ == other.value_;
}

ChatId::ChatId(std::int64_t value) : value_(value) {
    requireChatId(value_);
}

std::int64_t ChatId::value() const {
    return value_;
}

bool ChatId::operator==(const ChatId& other) const {
    return value_ == other.value_;
}

bool ChatId::operator!=(const ChatId& other) const {
    return !(*this == other);
}

MessageId::MessageId(std::int32_t value) : value_(value) {
    requireMessageId(value_);
}

std::int32_t MessageId::value() const {
    return value_;
}

bool MessageId::operator==(const MessageId& other) const {
    return value_ == other.value_;
}

bool MessageId::operator!=(const MessageId& other) const {
    return !(*this == other);
}

MessageText::MessageText(const std::string& value) : value_(value) {
    requireMessageText(value_);
}

const std::string& MessageText::value() const {
    return value_;
}

std::size_t MessageText::length() const {
    return value_.size();
}

bool MessageText::operator==(const MessageText& other) const {
    return value_ == other.value_;
}

ScreenCoordinate::ScreenCoordinate(int x, int y) : x_(x), y_(y) {
    requireCoordinate(x_, y_);
}

int ScreenCoordinate::x() const {
    return x_;
}

int ScreenCoordinate::y() const {
    return y_;
}

bool ScreenCoordinate::operator==(const ScreenCoordinate& other) const {
    return x_ == other.x_ && y_ == other.y_;
}

BoundingBox::BoundingBox(int left, int top, int right, int bottom)
    : BoundingBox(ScreenCoordinate(left, top), ScreenCoordinate(right, bottom)) {}

BoundingBox::BoundingBox(const ScreenCoordinate& top_left,
                         const ScreenCoordinate& bottom_right)
    : top_left_(top_left), bottom_right_(bottom_right) {
    requireBounds(top_left_, bottom_right_);
}

int BoundingBox::left() const {
    return top_left_.x();
}

int BoundingBox::top() const {
    return top_left_.y();
}

int BoundingBox::right() const {
    return bottom_right_.x();
}

int BoundingBox::bottom() const {
    return bottom_right_.y();
}

int BoundingBox::width() const {
    return right() - left() + 1;
}

int BoundingBox::height() const {
    return bottom() - top() + 1;
}

bool BoundingBox::contains(const ScreenCoordinate& coordinate) const {
    return coordinate.x() >= left() && coordinate.x() <= right() &&
           coordinate.y() >= top() && coordinate.y() <= bottom();
}

bool BoundingBox::operator==(const BoundingBox& other) const {
    return top_left_ == other.top_left_ && bottom_right_ == other.bottom_right_;
}

const GrayscaleColor GrayscaleColor::BLACK(0x00U);
const GrayscaleColor GrayscaleColor::DARK_GRAY(0x55U);
const GrayscaleColor GrayscaleColor::LIGHT_GRAY(0xAAU);
const GrayscaleColor GrayscaleColor::WHITE(0xFFU);

GrayscaleColor::GrayscaleColor(std::uint8_t value) : value_(value) {
    if (!isSupportedColor(value_)) {
        throw std::invalid_argument("Unsupported grayscale color");
    }
}

std::uint8_t GrayscaleColor::value() const {
    return value_;
}

bool GrayscaleColor::operator==(const GrayscaleColor& other) const {
    return value_ == other.value_;
}

bool GrayscaleColor::operator!=(const GrayscaleColor& other) const {
    return !(*this == other);
}
