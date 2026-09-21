#include "popup_collection.h"

#include "../graphics/canvas.h"

#include <algorithm>
#include <utility>

namespace ui {

bool PopupCollection::push(const PopupEntry& entry, PopupMode mode,
                           PopupTimestamp now) {
    if (mode == PopupMode::VISUAL_STACK) {
        return pushVisualStack(entry, now);
    }
    return pushSequentialQueue(entry, now);
}

bool PopupCollection::update(PopupTimestamp now, PopupMode mode) {
    if (mode == PopupMode::VISUAL_STACK) {
        return updateVisualStack(now);
    }
    return updateSequentialQueue(now);
}

std::size_t PopupCollection::visibleCount(PopupMode mode) const {
    if (mode == PopupMode::SEQUENTIAL_QUEUE) {
        return isActiveAt(0U) ? 1U : 0U;
    }
    std::size_t visible = 0U;
    for (std::size_t index = 0U; index < count_; ++index) {
        visible += isActiveAt(index) ? 1U : 0U;
    }
    return visible;
}

std::size_t PopupCollection::size() const {
    return count_;
}

std::size_t PopupCollection::pendingCount() const {
    return count_ > 0U ? count_ - 1U : 0U;
}

std::size_t PopupCollection::pendingCount(PopupMode mode) const {
    if (mode == PopupMode::VISUAL_STACK) {
        return 0U;
    }
    return pendingCount();
}

void PopupCollection::render(Canvas& canvas, const PopupLayout& layout,
                             PopupPosition position, PopupMode mode) {
    if (mode == PopupMode::VISUAL_STACK) {
        renderVisualStack(canvas, layout, position);
        return;
    }
    renderSequentialQueue(canvas, layout, position);
}

bool PopupCollection::pushVisualStack(const PopupEntry& entry,
                                       PopupTimestamp now) {
    shiftForNewest();
    entries_[0U].emplace(entry);
    entries_[0U]->activate(now);
    return true;
}

bool PopupCollection::pushSequentialQueue(const PopupEntry& entry,
                                           PopupTimestamp now) {
    if (count_ == CAPACITY) {
        return false;
    }
    entries_[count_].emplace(entry);
    ++count_;
    if (count_ == 1U) {
        activateFirst(now);
    }
    return true;
}

bool PopupCollection::updateVisualStack(PopupTimestamp now) {
    return removeExpiredFrom(0U, now);
}

bool PopupCollection::removeExpiredFrom(std::size_t index,
                                        PopupTimestamp now) {
    if (index >= count_) {
        return false;
    }
    if (!isExpiredAt(index, now)) {
        return removeExpiredFrom(index + 1U, now);
    }
    removeAt(index);
    removeExpiredFrom(index, now);
    return true;
}

bool PopupCollection::updateSequentialQueue(PopupTimestamp now) {
    if (count_ == 0U || !isActiveAt(0U) || !isExpiredAt(0U, now)) {
        return false;
    }
    removeAt(0U);
    if (count_ == 0U) {
        return true;
    }
    activateFirst(now);
    return true;
}

bool PopupCollection::isExpiredAt(std::size_t index,
                                  PopupTimestamp now) const {
    return isActiveAt(index) && entries_[index]->isExpired(now);
}

bool PopupCollection::isActiveAt(std::size_t index) const {
    return index < count_ && entries_[index].has_value() &&
           entries_[index]->isActive();
}

void PopupCollection::activateFirst(PopupTimestamp now) {
    if (count_ == 0U || !entries_[0U].has_value()) {
        return;
    }
    entries_[0U]->activate(now);
}

void PopupCollection::removeAt(std::size_t index) {
    if (index >= count_) {
        return;
    }
    for (std::size_t position = index; position + 1U < count_; ++position) {
        entries_[position] = std::move(entries_[position + 1U]);
    }
    entries_[count_ - 1U].reset();
    --count_;
}

void PopupCollection::shiftForNewest() {
    const std::size_t last = std::min(count_, CAPACITY - 1U);
    for (std::size_t position = last; position > 0U; --position) {
        entries_[position] = std::move(entries_[position - 1U]);
    }
    if (count_ < CAPACITY) {
        ++count_;
    }
}

void PopupCollection::renderVisualStack(Canvas& canvas,
                                         const PopupLayout& layout,
                                         PopupPosition position) {
    for (std::size_t index = count_; index > 0U; --index) {
        const std::size_t entry_index = index - 1U;
        renderActiveEntry(entry_index, canvas, layout, position, entry_index);
    }
}

void PopupCollection::renderSequentialQueue(Canvas& canvas,
                                             const PopupLayout& layout,
                                             PopupPosition position) {
    if (!isActiveAt(0U)) {
        return;
    }
    renderEntryAt(0U, canvas, layout, position, 0U);
}

bool PopupCollection::renderActiveEntry(std::size_t index, Canvas& canvas,
                                         const PopupLayout& layout,
                                         PopupPosition position,
                                         std::size_t visible_index) {
    if (!isActiveAt(index)) {
        return false;
    }
    renderEntryAt(index, canvas, layout, position, visible_index);
    return true;
}

void PopupCollection::renderEntryAt(std::size_t index, Canvas& canvas,
                                    const PopupLayout& layout,
                                    PopupPosition position,
                                    std::size_t visible_index) {
    const BoundingBox bounds = layout.calculateBounds(visible_index, position);
    entries_[index]->render(canvas, bounds);
}

} // namespace ui
