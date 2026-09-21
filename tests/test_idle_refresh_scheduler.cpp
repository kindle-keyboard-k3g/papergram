#include "test_framework.h"
#include "../src/graphics/idle_refresh_scheduler.h"
#include "mocks/mock_eink_controller.h"
#include <set>

TEST(idle_refresh_scheduler_waits_for_threshold) {
    MockEinkController controller;
    IdleRefreshScheduler scheduler(controller);
    scheduler.noteActivity(1000ULL);

    ASSERT_FALSE(scheduler.tick(1500ULL));
    ASSERT_FALSE(scheduler.tick(5999ULL));
    ASSERT_EQ(0U, controller.partialUpdateCount());

    ASSERT_TRUE(scheduler.tick(6000ULL)); // 1000 + 5000
    ASSERT_EQ(1U, controller.partialUpdateCount());
}

TEST(idle_refresh_scheduler_paces_tiles_by_step_interval) {
    MockEinkController controller;
    IdleRefreshScheduler scheduler(controller);
    scheduler.noteActivity(0ULL);

    // First tile at 5000ms
    ASSERT_TRUE(scheduler.tick(5000ULL));
    ASSERT_EQ(1U, controller.partialUpdateCount());

    // Too early for 2nd tile
    ASSERT_FALSE(scheduler.tick(5200ULL));
    ASSERT_EQ(1U, controller.partialUpdateCount());

    // 2nd tile at 5400ms (5000 + 400)
    ASSERT_TRUE(scheduler.tick(5400ULL));
    ASSERT_EQ(2U, controller.partialUpdateCount());
}

TEST(idle_refresh_scheduler_covers_all_sixteen_unique_tiles) {
    std::set<std::pair<int, int>> tiles;
    for (std::size_t i = 0; i < 16; ++i) {
        BoundingBox box = IdleRefreshScheduler::tileBoxAt(i);
        ASSERT_EQ(150, box.width());
        ASSERT_EQ(200, box.height());
        tiles.insert({box.left(), box.top()});
    }
    ASSERT_EQ(16U, tiles.size());
}

TEST(idle_refresh_scheduler_note_activity_resets_sweep) {
    MockEinkController controller;
    IdleRefreshScheduler scheduler(controller);
    scheduler.noteActivity(0ULL);

    ASSERT_TRUE(scheduler.tick(5000ULL));
    ASSERT_EQ(1U, scheduler.currentTileIndex());

    scheduler.noteActivity(5100ULL);
    ASSERT_EQ(0U, scheduler.currentTileIndex());

    // Must wait another 5000ms from 5100ms
    ASSERT_FALSE(scheduler.tick(5400ULL));
    ASSERT_FALSE(scheduler.tick(10099ULL));
    ASSERT_TRUE(scheduler.tick(10100ULL));
    ASSERT_EQ(2U, controller.partialUpdateCount());
}

TEST(idle_refresh_scheduler_full_sweep_resets_with_idle_threshold) {
    MockEinkController controller;
    IdleRefreshScheduler scheduler(controller);
    scheduler.noteActivity(0ULL);

    std::uint64_t time = 5000ULL;
    for (std::size_t i = 0; i < 16; ++i) {
        ASSERT_TRUE(scheduler.tick(time));
        if (i < 15) {
            time += 400ULL;
        }
    }
    ASSERT_EQ(16U, controller.partialUpdateCount());
    ASSERT_EQ(0U, scheduler.currentTileIndex());

    // After 16 tiles (last completed at `time`), it should wait for 5000ms idle threshold again
    ASSERT_FALSE(scheduler.tick(time + 100ULL));
    ASSERT_FALSE(scheduler.tick(time + 4999ULL));
    ASSERT_TRUE(scheduler.tick(time + 5000ULL));
    ASSERT_EQ(17U, controller.partialUpdateCount());
}
