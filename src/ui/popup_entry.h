#ifndef KINDLE_UI_POPUP_ENTRY_H
#define KINDLE_UI_POPUP_ENTRY_H

#include "popup_types.h"
#include "toast_notification.h"

#include <chrono>
#include <string>
#include <vector>

namespace ui {

class PopupEntry {
public:
    PopupEntry(const ToastNotification& card, const PopupDuration& duration);
    PopupEntry(const ToastNotification& card, const PopupTimestamp& created_at,
               const PopupDuration& duration, bool active = false);
    PopupEntry(const BoundingBox& bounds, const std::string& title,
               const std::vector<std::string>& lines,
               const PopupDuration& duration);

    bool isExpired(PopupTimestamp now) const;
    void activate(PopupTimestamp now);
    bool isActive() const;
    const ToastNotification& card() const;
    ToastNotification& card();
    const PopupTimestamp& createdAt() const;
    const PopupDuration& duration() const;

    void render(Canvas& canvas, const BoundingBox& bounds);

private:
    struct PopupData {
        ToastNotification card;
        PopupTimestamp created_at;
        PopupDuration duration;
        bool active;
    };

    PopupData data_;
};

} // namespace ui

#endif
