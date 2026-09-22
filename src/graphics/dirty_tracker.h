#ifndef KINDLE_GRAPHICS_DIRTY_TRACKER_H
#define KINDLE_GRAPHICS_DIRTY_TRACKER_H

#include "../domain/value_objects.h"

#include <cstddef>
#include <vector>

/**
 * @brief Collects rectangular regions that need display work.
 *
 * Marked regions that touch or are adjacent are merged, keeping the tracked
 * collection compact while preserving the union of all requested areas.
 */
class DirtyTracker {
public:
    /**
     * @brief Marks a region as dirty and merges touching regions.
     *
     * @param area Rectangle that needs processing.
     */
    void mark(const BoundingBox& area);

    /**
     * @brief Adds a region as an alias for mark().
     *
     * @param area Rectangle that needs processing.
     */
    void add(const BoundingBox& area);

    /**
     * @brief Checks whether any dirty region is stored.
     *
     * @return true when at least one region is present; otherwise false.
     */
    bool hasDirty() const;

    /**
     * @brief Returns the number of stored regions.
     *
     * @return Number of currently tracked regions.
     */
    std::size_t count() const;

    /**
     * @brief Returns one rectangle covering every stored region.
     *
     * @return Union bounding box of all tracked regions.
     * @throws std::logic_error If no dirty region is stored.
     */
    BoundingBox boundingBox() const;

    /**
     * @brief Returns a copy of the currently tracked regions.
     *
     * @return Vector containing each merged dirty region.
     */
    std::vector<BoundingBox> regions() const;

    /**
     * @brief Returns all regions and empties the tracker.
     *
     * @return Vector containing the regions present before consumption.
     */
    std::vector<BoundingBox> consume();

    /**
     * @brief Removes all tracked regions.
     */
    void clear();

private:
    std::vector<BoundingBox> regions_;

    static bool touches(const BoundingBox& first, const BoundingBox& second);
    static BoundingBox merged(const BoundingBox& first,
                              const BoundingBox& second);
    void mergeAt(std::size_t index, const BoundingBox& area);
};

#endif
