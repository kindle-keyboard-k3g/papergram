#include "idle_refresh_scheduler.h"
#include "util/debug_log.h"
#include <array>
#include <string>

namespace {
// 4x4 Grid geometry:
// cols (x): 0..149, 150..299, 300..449, 450..599 (width = 150)
// rows (y): 0..199, 200..399, 400..599, 600..799 (height = 200)
//
// Checkerboard / interleaved order across 16 tiles:
// Phase 1 (even sum col+row):
// (0,0)=0, (2,0)=2, (1,1)=5, (3,1)=7, (0,2)=8, (2,2)=10, (1,3)=13, (3,3)=15
// Phase 2 (odd sum col+row):
// (1,0)=1, (3,0)=3, (0,1)=4, (2,1)=6, (1,2)=9, (3,2)=11, (0,3)=12, (2,3)=14
constexpr std::array<std::size_t, 16> CHECKERBOARD_TILES = {
    0, 2, 5, 7, 8, 10, 13, 15,
    1, 3, 4, 6, 9, 11, 12, 14
};
}

IdleRefreshScheduler::IdleRefreshScheduler(IEinkController& controller)
    : controller_(controller),
      last_activity_ms_(0ULL),
      next_due_ms_(0ULL),
      tile_index_(0U),
      enabled_(true) {}

void IdleRefreshScheduler::noteActivity(std::uint64_t nowMs) {
    last_activity_ms_ = nowMs;
    next_due_ms_ = nowMs + IDLE_THRESHOLD_MS;
    tile_index_ = 0U;
}

void IdleRefreshScheduler::disable() {
    enabled_ = false;
}

bool IdleRefreshScheduler::isEnabled() const {
    return enabled_;
}

std::size_t IdleRefreshScheduler::currentTileIndex() const {
    return tile_index_;
}

BoundingBox IdleRefreshScheduler::tileBoxAt(std::size_t sequenceIndex) {
    const std::size_t clamped_index = sequenceIndex % TOTAL_TILES;
    const std::size_t tile_id = CHECKERBOARD_TILES[clamped_index];
    const int col = static_cast<int>(tile_id % 4);
    const int row = static_cast<int>(tile_id / 4);

    const int left = col * 150;
    const int right = left + 149;
    const int top = row * 200;
    const int bottom = top + 199;

    return BoundingBox(left, top, right, bottom);
}

bool IdleRefreshScheduler::tick(std::uint64_t nowMs) {
    if (!enabled_) {
        return false;
    }
    if (nowMs < next_due_ms_) {
        return false;
    }

    const BoundingBox box = tileBoxAt(tile_index_);
    DEBUG_LOG("Eink", "IdleRefreshScheduler DU tile " + std::to_string(tile_index_) +
              ": [" + std::to_string(box.left()) + "," + std::to_string(box.top()) +
              " - " + std::to_string(box.right()) + "," + std::to_string(box.bottom()) + "]");

    try {
        controller_.updateArea(box, false);
    } catch (...) {
        disable();
        return false;
    }

    advanceSweep(nowMs);
    return true;
}

void IdleRefreshScheduler::advanceSweep(std::uint64_t nowMs) {
    ++tile_index_;
    if (tile_index_ >= TOTAL_TILES) {
        tile_index_ = 0U;
        next_due_ms_ = nowMs + IDLE_THRESHOLD_MS;
    } else {
        next_due_ms_ = nowMs + STEP_INTERVAL_MS;
    }
}
