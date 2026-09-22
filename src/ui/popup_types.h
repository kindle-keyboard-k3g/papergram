#ifndef KINDLE_UI_POPUP_TYPES_H
#define KINDLE_UI_POPUP_TYPES_H

#include <chrono>

/** @brief Anchors popup cards at the top or bottom of the display. */
enum class PopupPosition {
    /** @brief Anchor cards to the bottom of the display. */
    BOTTOM,
    /** @brief Anchor cards to the top of the display. */
    TOP
};

/** @brief Selects how multiple popup cards are presented. */
enum class PopupMode {
    /** @brief Show active cards together in a visual stack. */
    VISUAL_STACK,
    /** @brief Show one card at a time in arrival order. */
    SEQUENTIAL_QUEUE
};

namespace ui {

/**
 * @brief Validates and stores the lifetime of a popup card.
 */
class PopupDuration {
public:
    /** @brief Creates a duration using the standard popup lifetime. */
    PopupDuration();

    /**
     * @brief Creates a duration from a millisecond value.
     * @param value Positive popup lifetime.
     * @throws std::invalid_argument If value is not positive.
     */
    explicit PopupDuration(std::chrono::milliseconds value);

    /**
     * @brief Returns the duration in milliseconds.
     * @return Stored popup lifetime.
     */
    std::chrono::milliseconds toMillis() const;

    /**
     * @brief Creates the standard popup duration.
     * @return Default popup lifetime.
     */
    static PopupDuration defaultValue();

private:
    std::chrono::milliseconds value_;
};

/**
 * @brief Represents a monotonic point in popup time.
 */
class PopupTimestamp {
public:
    /**
     * @brief Creates a timestamp from milliseconds.
     * @param value Millisecond value on the caller's time base.
     */
    explicit PopupTimestamp(std::chrono::milliseconds value);

    /**
     * @brief Returns the timestamp value in milliseconds.
     * @return Stored timestamp.
     */
    std::chrono::milliseconds millis() const;

    /**
     * @brief Adds a duration to this timestamp.
     * @param duration Duration to add.
     * @return Timestamp shifted forward by duration.
     */
    PopupTimestamp operator+(std::chrono::milliseconds duration) const;

    /**
     * @brief Subtracts a duration from this timestamp.
     * @param duration Duration to subtract.
     * @return Timestamp shifted backward by duration.
     */
    PopupTimestamp operator-(std::chrono::milliseconds duration) const;

    /**
     * @brief Computes the difference between two timestamps.
     * @param other Timestamp to subtract.
     * @return Duration between this timestamp and other.
     */
    std::chrono::milliseconds operator-(const PopupTimestamp& other) const;

    /**
     * @brief Compares whether this timestamp is at or after another.
     * @param other Timestamp to compare with.
     * @return True when this timestamp is greater than or equal to other.
     */
    bool operator>=(const PopupTimestamp& other) const;

    /**
     * @brief Compares whether this timestamp precedes another.
     * @param other Timestamp to compare with.
     * @return True when this timestamp is less than other.
     */
    bool operator<(const PopupTimestamp& other) const;

private:
    std::chrono::milliseconds value_;
};

} // namespace ui

#endif
