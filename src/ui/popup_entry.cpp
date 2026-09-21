#include "popup_entry.h"

namespace ui {

PopupEntry::PopupEntry(const ToastNotification& card,
                       const PopupDuration& duration)
    : PopupEntry(card, PopupTimestamp(std::chrono::milliseconds(0)), duration,
                 false) {}

PopupEntry::PopupEntry(const ToastNotification& card,
                       const PopupTimestamp& created_at,
                       const PopupDuration& duration, bool active)
    : data_{card, created_at, duration, active} {}

PopupEntry::PopupEntry(const BoundingBox& bounds, const std::string& title,
                       const std::vector<std::string>& lines,
                       const PopupDuration& duration)
    : PopupEntry(ToastNotification(bounds, title, lines), duration) {}

bool PopupEntry::isExpired(PopupTimestamp now) const {
    return now >= data_.created_at + data_.duration.toMillis();
}

void PopupEntry::activate(PopupTimestamp now) {
    data_.created_at = now;
    data_.active = true;
}

bool PopupEntry::isActive() const {
    return data_.active;
}

const ToastNotification& PopupEntry::card() const {
    return data_.card;
}

ToastNotification& PopupEntry::card() {
    return data_.card;
}

const PopupTimestamp& PopupEntry::createdAt() const {
    return data_.created_at;
}

const PopupDuration& PopupEntry::duration() const {
    return data_.duration;
}

void PopupEntry::render(Canvas& canvas, const BoundingBox& bounds) {
    data_.card.setBounds(bounds);
    data_.card.render(canvas);
}

} // namespace ui
