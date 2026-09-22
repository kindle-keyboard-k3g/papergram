#ifndef KINDLE_UI_POPUP_ENTRY_H
#define KINDLE_UI_POPUP_ENTRY_H

#include "popup_types.h"
#include "toast_notification.h"

#include <chrono>
#include <string>
#include <vector>

namespace ui {

/**
 * @brief Couples a toast card with its lifetime and activation timestamp.
 */
class PopupEntry {
public:
    /**
     * @brief Creates an inactive entry with a zero creation timestamp.
     * @param card Card to display.
     * @param duration Card lifetime after activation.
     */
    PopupEntry(const ToastNotification& card, const PopupDuration& duration);

    /**
     * @brief Creates an entry with explicit timing and activation state.
     * @param card Card to display.
     * @param created_at Timestamp from which expiration is measured.
     * @param duration Card lifetime after creation or activation.
     * @param active Whether the entry is initially active.
     */
    PopupEntry(const ToastNotification& card, const PopupTimestamp& created_at,
               const PopupDuration& duration, bool active = false);

    /**
     * @brief Creates an entry from card geometry and text.
     * @param bounds Initial card bounds.
     * @param title Card heading.
     * @param lines Card body lines.
     * @param duration Card lifetime after activation.
     */
    PopupEntry(const BoundingBox& bounds, const std::string& title,
               const std::vector<std::string>& lines,
               const PopupDuration& duration);

    /**
     * @brief Reports whether the entry has expired at a timestamp.
     * @param now Timestamp against which expiration is checked.
     * @return True when the active entry's lifetime has elapsed.
     */
    bool isExpired(PopupTimestamp now) const;

    /**
     * @brief Activates the entry and resets its expiration origin.
     * @param now Timestamp at which activation occurs.
     */
    void activate(PopupTimestamp now);

    /**
     * @brief Reports whether the entry is active.
     * @return True when the entry is eligible for display and expiry.
     */
    bool isActive() const;

    /**
     * @brief Returns the immutable toast card.
     * @return Read-only card.
     */
    const ToastNotification& card() const;

    /**
     * @brief Returns the mutable toast card.
     * @return Card that can be repositioned or otherwise updated.
     */
    ToastNotification& card();

    /**
     * @brief Returns the activation or creation timestamp.
     * @return Entry timestamp.
     */
    const PopupTimestamp& createdAt() const;

    /**
     * @brief Returns the configured card lifetime.
     * @return Entry duration.
     */
    const PopupDuration& duration() const;

    /**
     * @brief Positions and renders the toast card.
     * @param canvas Destination canvas.
     * @param bounds Bounds to apply before rendering.
     */
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
