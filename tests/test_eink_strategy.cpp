#include "test_framework.h"

#include "../src/graphics/dirty_tracker.h"
#include "../src/graphics/refresh_strategy.h"
#include "mocks/mock_eink_controller.h"

TEST(dirty_tracker_merges_overlapping_regions) {
    DirtyTracker tracker;
    tracker.mark(BoundingBox(10, 10, 20, 20));
    tracker.mark(BoundingBox(18, 18, 30, 30));

    ASSERT_EQ(1U, tracker.count());
    ASSERT_EQ(BoundingBox(10, 10, 30, 30), tracker.boundingBox());
}

TEST(dirty_tracker_keeps_disjoint_regions_separate) {
    DirtyTracker tracker;
    tracker.mark(BoundingBox(1, 1, 2, 2));
    tracker.mark(BoundingBox(10, 10, 12, 12));

    ASSERT_EQ(2U, tracker.count());
    ASSERT_TRUE(tracker.hasDirty());
}

TEST(typing_refresh_updates_the_dirty_area_with_du) {
    MockEinkController controller;
    TypingRefresh strategy(controller);
    DirtyTracker tracker;
    tracker.mark(BoundingBox(20, 30, 40, 50));

    strategy.refresh(tracker);

    ASSERT_EQ(1U, controller.partialUpdateCount());
    ASSERT_EQ(BoundingBox(20, 30, 40, 50), controller.callAt(0).area);
    ASSERT_FALSE(controller.callAt(0).is_full_refresh);
}

TEST(full_refresh_flashes_after_fifteen_keystrokes) {
    MockEinkController controller;
    FullRefresh strategy(controller);
    DirtyTracker tracker;
    tracker.mark(BoundingBox(20, 30, 40, 50));

    for (int keystroke = 0; keystroke < 14; ++keystroke) {
        strategy.refresh(tracker);
    }
    ASSERT_EQ(14U, controller.partialUpdateCount());
    ASSERT_EQ(0U, controller.fullRefreshCount());

    strategy.refresh(tracker);

    ASSERT_EQ(1U, controller.fullRefreshCount());
    ASSERT_EQ(14U, controller.partialUpdateCount());
}

TEST(full_refresh_flashes_immediately_when_screen_changes) {
    MockEinkController controller;
    FullRefresh strategy(controller);
    DirtyTracker tracker;
    tracker.mark(BoundingBox(1, 1, 2, 2));

    strategy.refresh(tracker, true);

    ASSERT_EQ(1U, controller.fullRefreshCount());
    ASSERT_EQ(0U, controller.partialUpdateCount());
}
