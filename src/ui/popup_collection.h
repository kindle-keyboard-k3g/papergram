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

/**
 * @brief Stores a bounded collection of popup entries in stack or queue mode.
 */
class PopupCollection {
public:
    /** @brief Maximum number of entries retained by the collection. */
    static constexpr std::size_t CAPACITY = 4U;

    /**
     * @brief Adds an entry according to the selected presentation mode.
     * @param entry Entry to add.
     * @param mode Stack or sequential-queue behavior.
     * @param now Timestamp used to activate the entry.
     * @return True when the entry is retained; false when a queue is full.
     */
    bool push(const PopupEntry& entry, PopupMode mode, PopupTimestamp now);

    /**
     * @brief Removes expired entries and advances queued entries.
     * @param now Current popup timestamp.
     * @param mode Stack or sequential-queue behavior.
     * @return True when the collection changed.
     */
    bool update(PopupTimestamp now, PopupMode mode);

    /**
     * @brief Counts entries currently eligible for rendering.
     * @param mode Stack or sequential-queue behavior.
     * @return Number of visible entries for mode.
     */
    std::size_t visibleCount(PopupMode mode) const;

    /**
     * @brief Returns the number of retained entries.
     * @return Collection size.
     */
    std::size_t size() const;

    /**
     * @brief Returns entries waiting behind the active entry.
     * @return Pending entry count.
     */
    std::size_t pendingCount() const;

    /**
     * @brief Returns pending entries for a presentation mode.
     * @param mode Stack or sequential-queue behavior.
     * @return Pending count; zero for visual-stack mode.
     */
    std::size_t pendingCount(PopupMode mode) const;

    /**
     * @brief Renders active entries using the supplied layout.
     * @param canvas Destination canvas.
     * @param layout Layout used to calculate card bounds.
     * @param position Top or bottom card anchor.
     * @param mode Stack or sequential-queue behavior.
     */
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
