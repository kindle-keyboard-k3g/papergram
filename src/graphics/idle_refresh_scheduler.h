#ifndef KINDLE_GRAPHICS_IDLE_REFRESH_SCHEDULER_H
#define KINDLE_GRAPHICS_IDLE_REFRESH_SCHEDULER_H

#include "../hal/eink_controller.h"
#include "../domain/value_objects.h"
#include <cstddef>
#include <cstdint>

/**
 * @brief Schedules low-priority tiled refreshes while the device is idle.
 *
 * The 600 by 800 display is swept as sixteen 150 by 200 tiles in an
 * interleaved checkerboard order. User activity restarts the idle delay and
 * the sweep from its first tile.
 */
class IdleRefreshScheduler {
public:
    /** @brief Idle duration required before a sweep starts, in milliseconds. */
    static constexpr std::uint64_t IDLE_THRESHOLD_MS = 5000ULL;

    /** @brief Delay between tiles in an active sweep, in milliseconds. */
    static constexpr std::uint64_t STEP_INTERVAL_MS = 400ULL;

    /** @brief Number of tiles in one complete display sweep. */
    static constexpr std::size_t TOTAL_TILES = 16U;

    /**
     * @brief Creates an enabled scheduler backed by an e-ink controller.
     *
     * @param controller Controller used to issue tile refreshes.
     */
    explicit IdleRefreshScheduler(IEinkController& controller);

    /**
     * @brief Records user activity and postpones idle refreshing.
     *
     * @param nowMs Current monotonic time in milliseconds.
     */
    void noteActivity(std::uint64_t nowMs);

    /**
     * @brief Performs one due tile refresh, if scheduling permits it.
     *
     * A controller failure disables the scheduler and is not propagated.
     *
     * @param nowMs Current monotonic time in milliseconds.
     * @return true when a tile update was issued; otherwise false.
     */
    bool tick(std::uint64_t nowMs);

    /**
     * @brief Permanently disables future scheduled tile refreshes.
     */
    void disable();

    /**
     * @brief Checks whether scheduled tile refreshing is enabled.
     *
     * @return true when tick() may issue updates; otherwise false.
     */
    bool isEnabled() const;

    /**
     * @brief Returns the current position in the checkerboard sweep.
     *
     * @return Zero-based sequence index of the next tile to refresh.
     */
    std::size_t currentTileIndex() const;

    /**
     * @brief Calculates a display tile for a sweep sequence position.
     *
     * Sequence positions wrap modulo TOTAL_TILES.
     *
     * @param sequenceIndex Zero-based position in the sweep.
     * @return Bounding box of the corresponding 150 by 200 tile.
     */
    static BoundingBox tileBoxAt(std::size_t sequenceIndex);

private:
    IEinkController& controller_;
    std::uint64_t last_activity_ms_;
    std::uint64_t next_due_ms_;
    std::size_t tile_index_;
    bool enabled_;

    void advanceSweep(std::uint64_t nowMs);
};

#endif
