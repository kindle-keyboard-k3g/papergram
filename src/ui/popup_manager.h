#pragma once

#include "popup_collection.h"
#include "popup_layout.h"
#include "popup_types.h"
#include "../domain/message.h"

#include <optional>
#include <string>
#include <vector>

namespace ui {

/**
 * @brief Manages popup creation, timing, placement, and redraw notifications.
 */
class PopupManager {
public:
    /** @brief Creates an empty manager using visual-stack bottom popups. */
    PopupManager();

    /**
     * @brief Adds a titled popup to the configured collection.
     * @param title Popup heading.
     * @param lines Popup body lines.
     * @param duration Popup lifetime.
     * @return True when the popup is accepted; false when capacity is full.
     */
    bool show(const std::string& title,
              const std::vector<std::string>& lines,
              PopupDuration duration = PopupDuration::defaultValue());

    /**
     * @brief Shows an incoming message unless it should be suppressed.
     * @param message Incoming or outgoing message to inspect.
     * @param active_chat_id Currently open chat, if any.
     * @param muted Whether notifications are muted.
     * @return True when a popup is created; false when filtered or rejected.
     */
    bool notifyIncomingMessage(const Message& message,
                               const std::optional<ChatId>& active_chat_id,
                               bool muted = false);

    /**
     * @brief Advances popup timing and removes entries that expired.
     * @param now Current popup timestamp.
     * @return True when the collection changed and a redraw is needed.
     */
    bool update(PopupTimestamp now);

    /**
     * @brief Renders all currently visible popups.
     * @param canvas Destination canvas.
     */
    void render(Canvas& canvas);

    /**
     * @brief Selects how multiple popups are presented.
     * @param mode New popup presentation mode.
     */
    void setMode(PopupMode mode);

    /**
     * @brief Selects the edge of the screen where popups are anchored.
     * @param position New popup position.
     */
    void setPosition(PopupPosition position);

    /**
     * @brief Returns the current presentation mode.
     * @return Configured popup mode.
     */
    PopupMode mode() const;

    /**
     * @brief Returns the current popup anchor position.
     * @return Configured popup position.
     */
    PopupPosition position() const;

    /**
     * @brief Returns the number of visible popups.
     * @return Visible popup count.
     */
    std::size_t visibleCount() const;

    /**
     * @brief Returns the number of queued popups awaiting display.
     * @return Pending popup count.
     */
    std::size_t pendingCount() const;

    /**
     * @brief Reports whether a redraw has been requested.
     * @return True when popup state changed since the last acknowledgement.
     */
    bool needsRedraw() const;

    /**
     * @brief Returns the most recent popup damage area, when available.
     * @return Optional bounds that should be redrawn.
     */
    const std::optional<BoundingBox>& damageArea() const;

    /** @brief Clears the pending redraw flag after rendering. */
    void acknowledgeRedraw();

private:
    struct State {
        PopupCollection collection;
        PopupLayout layout;
        PopupPosition position{PopupPosition::BOTTOM};
        PopupMode mode{PopupMode::VISUAL_STACK};
        PopupTimestamp current_time{std::chrono::milliseconds(0)};
        bool needs_redraw{false};
        std::optional<BoundingBox> damage_area;
    };

    void markRedraw();
    bool isForActiveChat(const Message& message,
                         const std::optional<ChatId>& active_chat_id) const;

    State state_;
};

} // namespace ui
