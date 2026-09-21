#pragma once

#include "popup_collection.h"
#include "popup_layout.h"
#include "popup_types.h"
#include "../domain/message.h"

#include <optional>
#include <string>
#include <vector>

namespace ui {

class PopupManager {
public:
    PopupManager();

    bool show(const std::string& title,
              const std::vector<std::string>& lines,
              PopupDuration duration = PopupDuration::defaultValue());

    bool notifyIncomingMessage(const Message& message,
                               const std::optional<ChatId>& active_chat_id,
                               bool muted = false);

    bool update(PopupTimestamp now);
    void render(Canvas& canvas);

    void setMode(PopupMode mode);
    void setPosition(PopupPosition position);

    PopupMode mode() const;
    PopupPosition position() const;
    std::size_t visibleCount() const;
    std::size_t pendingCount() const;

    bool needsRedraw() const;
    const std::optional<BoundingBox>& damageArea() const;
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
