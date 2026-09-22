#ifndef KINDLE_GRAPHICS_DIFF_TRACKER_H
#define KINDLE_GRAPHICS_DIFF_TRACKER_H

#include "dirty_tracker.h"
#include <cstddef>
#include <cstdint>

/**
 * @brief Tracks changed and dark-to-white regions between two pixel buffers.
 *
 * A row's contiguous changes are recorded as dirty regions. If a result would
 * exceed MAX_REGIONS, the corresponding tracker is replaced by one full-buffer
 * region.
 */
class BufferDiffTracker {
public:
    /** @brief Maximum number of regions retained before coalescing to full size. */
    static constexpr std::size_t MAX_REGIONS = 64U;

    /**
     * @brief Creates an empty buffer diff tracker.
     */
    BufferDiffTracker();

    /**
     * @brief Compares two row-major grayscale buffers.
     *
     * Existing results are cleared before comparison. Null buffers or zero
     * dimensions produce an empty result.
     *
     * @param previous Earlier pixel buffer.
     * @param current Later pixel buffer.
     * @param width Buffer width in pixels.
     * @param height Buffer height in pixels.
     */
    void compare(const std::uint8_t* previous,
                 const std::uint8_t* current,
                 std::size_t width = 600U,
                 std::size_t height = 800U);

    /**
     * @brief Returns regions containing any changed pixel.
     *
     * @return Read-only tracker of all changed regions.
     */
    const DirtyTracker& changed() const;

    /**
     * @brief Returns regions changed from non-white to white.
     *
     * @return Read-only tracker of dark-to-white regions.
     */
    const DirtyTracker& darkToWhite() const;

    /**
     * @brief Checks whether any pixel changed in the last comparison.
     *
     * @return true when changed() contains at least one region.
     */
    bool hasChanges() const;

    /**
     * @brief Checks whether any dark-to-white transition was found.
     *
     * @return true when darkToWhite() contains at least one region.
     */
    bool hasDarkToWhite() const;

    /**
     * @brief Discards all changed and dark-to-white regions.
     */
    void clear();

private:
    DirtyTracker changed_;
    DirtyTracker dark_to_white_;

    void processRow(const std::uint8_t* previous,
                    const std::uint8_t* current,
                    int y,
                    std::size_t width);
    void checkCap(DirtyTracker& tracker, std::size_t width, std::size_t height);
};

#endif
