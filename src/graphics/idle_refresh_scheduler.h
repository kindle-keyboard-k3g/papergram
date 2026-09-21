#ifndef KINDLE_GRAPHICS_IDLE_REFRESH_SCHEDULER_H
#define KINDLE_GRAPHICS_IDLE_REFRESH_SCHEDULER_H

#include "../hal/eink_controller.h"
#include "../domain/value_objects.h"
#include <cstddef>
#include <cstdint>

class IdleRefreshScheduler {
public:
    static constexpr std::uint64_t IDLE_THRESHOLD_MS = 5000ULL;
    static constexpr std::uint64_t STEP_INTERVAL_MS = 400ULL;
    static constexpr std::size_t TOTAL_TILES = 16U;

    explicit IdleRefreshScheduler(IEinkController& controller);

    void noteActivity(std::uint64_t nowMs);
    bool tick(std::uint64_t nowMs);
    void disable();
    bool isEnabled() const;
    std::size_t currentTileIndex() const;

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
