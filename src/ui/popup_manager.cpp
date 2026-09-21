#include "popup_manager.h"

#include "../graphics/canvas.h"

namespace ui {

PopupManager::PopupManager() = default;

bool PopupManager::show(const std::string& title,
                        const std::vector<std::string>& lines,
                        PopupDuration duration) {
    const BoundingBox bounds = state_.layout.calculateBounds(0U, state_.position);
    const PopupEntry entry(bounds, title, lines, duration);
    if (!state_.collection.push(entry, state_.mode, state_.current_time)) {
        return false;
    }
    markRedraw();
    return true;
}

bool PopupManager::notifyIncomingMessage(
    const Message& message, const std::optional<ChatId>& active_chat_id,
    bool muted) {
    if (message.isOutgoing()) {
        return false;
    }
    if (isForActiveChat(message, active_chat_id)) {
        return false;
    }
    if (muted) {
        return false;
    }
    return show(message.sender(), {message.text().value()});
}

bool PopupManager::update(PopupTimestamp now) {
    state_.current_time = now;
    if (!state_.collection.update(now, state_.mode)) {
        return false;
    }
    markRedraw();
    return true;
}

void PopupManager::render(Canvas& canvas) {
    if (visibleCount() == 0U) {
        return;
    }
    state_.collection.render(canvas, state_.layout, state_.position, state_.mode);
}

void PopupManager::setMode(PopupMode mode) {
    if (state_.mode == mode) {
        return;
    }
    state_.mode = mode;
    markRedraw();
}

void PopupManager::setPosition(PopupPosition position) {
    if (state_.position == position) {
        return;
    }
    state_.position = position;
    markRedraw();
}

PopupMode PopupManager::mode() const {
    return state_.mode;
}

PopupPosition PopupManager::position() const {
    return state_.position;
}

std::size_t PopupManager::visibleCount() const {
    return state_.collection.visibleCount(state_.mode);
}

std::size_t PopupManager::pendingCount() const {
    return state_.collection.pendingCount(state_.mode);
}

bool PopupManager::needsRedraw() const {
    return state_.needs_redraw;
}

const std::optional<BoundingBox>& PopupManager::damageArea() const {
    return state_.damage_area;
}

void PopupManager::acknowledgeRedraw() {
    state_.needs_redraw = false;
}

void PopupManager::markRedraw() {
    state_.needs_redraw = true;
    state_.damage_area = state_.layout.calculateDamageArea(visibleCount(),
                                                            state_.position);
}

bool PopupManager::isForActiveChat(
    const Message& message, const std::optional<ChatId>& active_chat_id) const {
    return active_chat_id.has_value() && message.chatId() == *active_chat_id;
}

} // namespace ui
