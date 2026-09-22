#ifndef KINDLE_DOMAIN_VALUE_OBJECTS_H
#define KINDLE_DOMAIN_VALUE_OBJECTS_H

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @brief Validated international phone number in E.164 notation.
 */
class PhoneNumber {
public:
    /**
     * @brief Creates a phone number after validating its E.164 representation.
     * @param value Phone number beginning with a plus sign and country code.
     * @throws std::invalid_argument If the value is empty, too long, malformed,
     *         or does not start with a non-zero digit after the plus sign.
     */
    explicit PhoneNumber(const std::string& value);

    /**
     * @brief Returns the normalized phone number text.
     * @return The stored E.164 phone number.
     */
    const std::string& value() const;

    /**
     * @brief Compares two phone numbers for equality.
     * @param other Phone number to compare with.
     * @return true when both numbers contain the same text; otherwise false.
     */
    bool operator==(const PhoneNumber& other) const;

private:
    std::string value_;
};

/**
 * @brief Validated five-digit Telegram authentication code.
 */
class AuthCode {
public:
    /**
     * @brief Creates an authentication code.
     * @param value Exactly five decimal digits.
     * @throws std::invalid_argument If the value is not exactly five digits.
     */
    explicit AuthCode(const std::string& value);

    /**
     * @brief Returns the authentication code text.
     * @return The stored five-digit code.
     */
    const std::string& value() const;

    /**
     * @brief Compares two authentication codes for equality.
     * @param other Authentication code to compare with.
     * @return true when both codes contain the same text; otherwise false.
     */
    bool operator==(const AuthCode& other) const;

private:
    std::string value_;
};

/**
 * @brief Validated cloud password used for Telegram two-factor authentication.
 */
class CloudPassword {
public:
    /**
     * @brief Creates a cloud password.
     * @param value Non-empty password of at most 256 bytes.
     * @throws std::invalid_argument If the password is empty or exceeds 256 bytes.
     */
    explicit CloudPassword(const std::string& value);

    /**
     * @brief Returns the cloud password text.
     * @return The stored password.
     */
    const std::string& value() const;

    /**
     * @brief Compares two cloud passwords for equality.
     * @param other Cloud password to compare with.
     * @return true when both passwords contain the same text; otherwise false.
     */
    bool operator==(const CloudPassword& other) const;

private:
    std::string value_;
};

/**
 * @brief Validated signed Telegram chat identifier.
 */
class ChatId {
public:
    /**
     * @brief Creates a chat identifier.
     * @param value Non-zero Telegram chat identifier.
     * @throws std::invalid_argument If value is zero.
     */
    explicit ChatId(std::int64_t value);

    /**
     * @brief Returns the numeric chat identifier.
     * @return The stored non-zero identifier.
     */
    std::int64_t value() const;

    /**
     * @brief Compares two chat identifiers for equality.
     * @param other Chat identifier to compare with.
     * @return true when both identifiers are equal; otherwise false.
     */
    bool operator==(const ChatId& other) const;

    /**
     * @brief Compares two chat identifiers for inequality.
     * @param other Chat identifier to compare with.
     * @return true when the identifiers differ; otherwise false.
     */
    bool operator!=(const ChatId& other) const;

private:
    std::int64_t value_;
};

/**
 * @brief Validated signed Telegram message identifier.
 */
class MessageId {
public:
    /**
     * @brief Creates a message identifier.
     * @param value Positive Telegram message identifier.
     * @throws std::invalid_argument If value is not positive.
     */
    explicit MessageId(std::int32_t value);

    /**
     * @brief Returns the numeric message identifier.
     * @return The stored positive identifier.
     */
    std::int32_t value() const;

    /**
     * @brief Compares two message identifiers for equality.
     * @param other Message identifier to compare with.
     * @return true when both identifiers are equal; otherwise false.
     */
    bool operator==(const MessageId& other) const;

    /**
     * @brief Compares two message identifiers for inequality.
     * @param other Message identifier to compare with.
     * @return true when the identifiers differ; otherwise false.
     */
    bool operator!=(const MessageId& other) const;

private:
    std::int32_t value_;
};

/**
 * @brief Validated UTF-8 message body.
 */
class MessageText {
public:
    /**
     * @brief Creates message text.
     * @param value Non-empty valid UTF-8 text of at most 4096 bytes.
     * @throws std::invalid_argument If the text is empty, too long, or invalid UTF-8.
     */
    explicit MessageText(const std::string& value);

    /**
     * @brief Returns the message text.
     * @return The stored UTF-8 text.
     */
    const std::string& value() const;

    /**
     * @brief Returns the message text length in bytes.
     * @return Number of bytes in the stored UTF-8 text.
     */
    std::size_t length() const;

    /**
     * @brief Compares two message bodies for equality.
     * @param other Message text to compare with.
     * @return true when both bodies contain the same bytes; otherwise false.
     */
    bool operator==(const MessageText& other) const;

private:
    std::string value_;
};

/**
 * @brief Validated coordinate on the 600-by-800 Kindle display.
 */
class ScreenCoordinate {
public:
    /**
     * @brief Creates a screen coordinate.
     * @param x Horizontal pixel coordinate from 0 through 600.
     * @param y Vertical pixel coordinate from 0 through 800.
     * @throws std::out_of_range If either coordinate is outside the display.
     */
    ScreenCoordinate(int x, int y);

    /**
     * @brief Returns the horizontal pixel coordinate.
     * @return The x coordinate.
     */
    int x() const;

    /**
     * @brief Returns the vertical pixel coordinate.
     * @return The y coordinate.
     */
    int y() const;

    /**
     * @brief Compares two screen coordinates for equality.
     * @param other Coordinate to compare with.
     * @return true when both coordinates match; otherwise false.
     */
    bool operator==(const ScreenCoordinate& other) const;

private:
    int x_;
    int y_;
};

/**
 * @brief Inclusive rectangular region on the Kindle display.
 */
class BoundingBox {
public:
    /**
     * @brief Creates a bounding box from its inclusive pixel limits.
     * @param left Leftmost horizontal coordinate.
     * @param top Topmost vertical coordinate.
     * @param right Rightmost horizontal coordinate.
     * @param bottom Bottommost vertical coordinate.
     * @throws std::out_of_range If any coordinate is outside the display.
     * @throws std::invalid_argument If the right or bottom edge precedes its left
     *         or top edge.
     */
    BoundingBox(int left, int top, int right, int bottom);

    /**
     * @brief Creates a bounding box from its two inclusive corners.
     * @param top_left Top-left corner of the region.
     * @param bottom_right Bottom-right corner of the region.
     * @throws std::invalid_argument If the corners are out of order.
     */
    BoundingBox(const ScreenCoordinate& top_left,
                const ScreenCoordinate& bottom_right);

    /**
     * @brief Returns the left edge coordinate.
     * @return Inclusive left edge.
     */
    int left() const;

    /**
     * @brief Returns the top edge coordinate.
     * @return Inclusive top edge.
     */
    int top() const;

    /**
     * @brief Returns the right edge coordinate.
     * @return Inclusive right edge.
     */
    int right() const;

    /**
     * @brief Returns the bottom edge coordinate.
     * @return Inclusive bottom edge.
     */
    int bottom() const;

    /**
     * @brief Returns the inclusive width of the region in pixels.
     * @return Number of horizontal pixels covered by the box.
     */
    int width() const;

    /**
     * @brief Returns the inclusive height of the region in pixels.
     * @return Number of vertical pixels covered by the box.
     */
    int height() const;

    /**
     * @brief Tests whether a coordinate lies inside this box.
     * @param coordinate Coordinate to test.
     * @return true when the coordinate is within both inclusive edges; otherwise false.
     */
    bool contains(const ScreenCoordinate& coordinate) const;

    /**
     * @brief Compares two bounding boxes for equality.
     * @param other Bounding box to compare with.
     * @return true when both corners match; otherwise false.
     */
    bool operator==(const BoundingBox& other) const;

private:
    ScreenCoordinate top_left_;
    ScreenCoordinate bottom_right_;
};

/**
 * @brief Supported four-tone grayscale color for the e-ink display.
 */
class GrayscaleColor {
public:
    /** @brief Black display tone. */
    static const GrayscaleColor BLACK;

    /** @brief Dark-gray display tone. */
    static const GrayscaleColor DARK_GRAY;

    /** @brief Light-gray display tone. */
    static const GrayscaleColor LIGHT_GRAY;

    /** @brief White display tone. */
    static const GrayscaleColor WHITE;

    /**
     * @brief Creates a supported grayscale color.
     * @param value One of the four supported tone values: 0x00, 0x55, 0xAA, or 0xFF.
     * @throws std::invalid_argument If value is not one of the supported tones.
     */
    explicit GrayscaleColor(std::uint8_t value);

    /**
     * @brief Returns the raw grayscale value.
     * @return The display tone as an 8-bit value.
     */
    std::uint8_t value() const;

    /**
     * @brief Compares two grayscale colors for equality.
     * @param other Color to compare with.
     * @return true when both colors have the same tone; otherwise false.
     */
    bool operator==(const GrayscaleColor& other) const;

    /**
     * @brief Compares two grayscale colors for inequality.
     * @param other Color to compare with.
     * @return true when the colors have different tones; otherwise false.
     */
    bool operator!=(const GrayscaleColor& other) const;

private:
    std::uint8_t value_;
};

#endif
