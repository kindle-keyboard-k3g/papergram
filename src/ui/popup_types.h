#ifndef KINDLE_UI_POPUP_TYPES_H
#define KINDLE_UI_POPUP_TYPES_H

#include <chrono>

enum class PopupPosition { BOTTOM, TOP };
enum class PopupMode { VISUAL_STACK, SEQUENTIAL_QUEUE };

namespace ui {

class PopupDuration {
public:
    PopupDuration();
    explicit PopupDuration(std::chrono::milliseconds value);

    std::chrono::milliseconds toMillis() const;
    static PopupDuration defaultValue();

private:
    std::chrono::milliseconds value_;
};

class PopupTimestamp {
public:
    explicit PopupTimestamp(std::chrono::milliseconds value);

    std::chrono::milliseconds millis() const;
    PopupTimestamp operator+(std::chrono::milliseconds duration) const;
    PopupTimestamp operator-(std::chrono::milliseconds duration) const;
    std::chrono::milliseconds operator-(const PopupTimestamp& other) const;
    bool operator>=(const PopupTimestamp& other) const;
    bool operator<(const PopupTimestamp& other) const;

private:
    std::chrono::milliseconds value_;
};

} // namespace ui

#endif
