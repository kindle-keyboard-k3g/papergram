#ifndef KINDLE_UI_POPUP_COLLECTION_H
#define KINDLE_UI_POPUP_COLLECTION_H

#include "popup_entry.h"
#include "popup_layout.h"
#include "popup_types.h"

#include <array>
#include <cstddef>
#include <optional>

class Canvas;

namespace ui {

class PopupCollection {
public:
    static constexpr std::size_t CAPACITY = 4U;

    bool push(const PopupEntry& entry, PopupMode mode, PopupTimestamp now);
    bool update(PopupTimestamp now, PopupMode mode);
    std::size_t visibleCount(PopupMode mode) const;
    std::size_t size() const;
    std::size_t pendingCount() const;
    std::size_t pendingCount(PopupMode mode) const;
    void render(Canvas& canvas, const PopupLayout& layout,
                PopupPosition position, PopupMode mode);

private:
    using EntrySlots = std::array<std::optional<PopupEntry>, CAPACITY>;

    EntrySlots entries_;
    std::size_t count_{0U};

    bool pushVisualStack(const PopupEntry& entry, PopupTimestamp now);
    bool pushSequentialQueue(const PopupEntry& entry, PopupTimestamp now);
    bool updateVisualStack(PopupTimestamp now);
    bool updateSequentialQueue(PopupTimestamp now);
    bool removeExpiredFrom(std::size_t index, PopupTimestamp now);
    bool isExpiredAt(std::size_t index, PopupTimestamp now) const;
    bool isActiveAt(std::size_t index) const;
    void activateFirst(PopupTimestamp now);
    void removeAt(std::size_t index);
    void shiftForNewest();
    void renderVisualStack(Canvas& canvas, const PopupLayout& layout,
                           PopupPosition position);
    void renderSequentialQueue(Canvas& canvas, const PopupLayout& layout,
                               PopupPosition position);
    bool renderActiveEntry(std::size_t index, Canvas& canvas,
                           const PopupLayout& layout, PopupPosition position,
                           std::size_t visible_index);
    void renderEntryAt(std::size_t index, Canvas& canvas,
                       const PopupLayout& layout, PopupPosition position,
                       std::size_t visible_index);
};

} // namespace ui

#endif
