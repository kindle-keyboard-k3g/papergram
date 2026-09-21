#ifndef KINDLE_DOMAIN_VALUE_OBJECTS_H
#define KINDLE_DOMAIN_VALUE_OBJECTS_H

#include <cstddef>
#include <cstdint>
#include <string>

class PhoneNumber {
public:
    explicit PhoneNumber(const std::string& value);

    const std::string& value() const;
    bool operator==(const PhoneNumber& other) const;

private:
    std::string value_;
};

class AuthCode {
public:
    explicit AuthCode(const std::string& value);

    const std::string& value() const;
    bool operator==(const AuthCode& other) const;

private:
    std::string value_;
};

class CloudPassword {
public:
    explicit CloudPassword(const std::string& value);

    const std::string& value() const;
    bool operator==(const CloudPassword& other) const;

private:
    std::string value_;
};

class ChatId {
public:
    explicit ChatId(std::int64_t value);

    std::int64_t value() const;
    bool operator==(const ChatId& other) const;
    bool operator!=(const ChatId& other) const;

private:
    std::int64_t value_;
};

class MessageId {
public:
    explicit MessageId(std::int32_t value);

    std::int32_t value() const;
    bool operator==(const MessageId& other) const;
    bool operator!=(const MessageId& other) const;

private:
    std::int32_t value_;
};

class MessageText {
public:
    explicit MessageText(const std::string& value);

    const std::string& value() const;
    std::size_t length() const;
    bool operator==(const MessageText& other) const;

private:
    std::string value_;
};

class ScreenCoordinate {
public:
    ScreenCoordinate(int x, int y);

    int x() const;
    int y() const;
    bool operator==(const ScreenCoordinate& other) const;

private:
    int x_;
    int y_;
};

class BoundingBox {
public:
    BoundingBox(int left, int top, int right, int bottom);
    BoundingBox(const ScreenCoordinate& top_left,
                const ScreenCoordinate& bottom_right);

    int left() const;
    int top() const;
    int right() const;
    int bottom() const;
    int width() const;
    int height() const;
    bool contains(const ScreenCoordinate& coordinate) const;
    bool operator==(const BoundingBox& other) const;

private:
    ScreenCoordinate top_left_;
    ScreenCoordinate bottom_right_;
};

class GrayscaleColor {
public:
    static const GrayscaleColor BLACK;
    static const GrayscaleColor DARK_GRAY;
    static const GrayscaleColor LIGHT_GRAY;
    static const GrayscaleColor WHITE;

    explicit GrayscaleColor(std::uint8_t value);

    std::uint8_t value() const;
    bool operator==(const GrayscaleColor& other) const;
    bool operator!=(const GrayscaleColor& other) const;

private:
    std::uint8_t value_;
};

#endif
