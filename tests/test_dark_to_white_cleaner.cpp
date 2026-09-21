#include "test_framework.h"
#include "../src/graphics/dark_to_white_cleaner.h"
#include "mocks/mock_eink_controller.h"

TEST(dark_to_white_cleaner_is_noop_when_empty) {
    MockEinkController controller;
    DarkToWhiteCleaner cleaner(controller);
    DirtyTracker tracker;

    cleaner.clean(tracker);
    ASSERT_EQ(0U, controller.partialUpdateCount());
}

TEST(dark_to_white_cleaner_issues_du_pass_for_regions) {
    MockEinkController controller;
    DarkToWhiteCleaner cleaner(controller);
    DirtyTracker tracker;
    tracker.mark(BoundingBox(10, 20, 30, 40));
    tracker.mark(BoundingBox(100, 150, 200, 250));

    cleaner.clean(tracker);
    ASSERT_EQ(2U, controller.partialUpdateCount());
    ASSERT_EQ(BoundingBox(10, 20, 30, 40), controller.callAt(0).area);
    ASSERT_FALSE(controller.callAt(0).is_full_refresh);
    ASSERT_EQ(BoundingBox(100, 150, 200, 250), controller.callAt(1).area);
    ASSERT_FALSE(controller.callAt(1).is_full_refresh);
}
