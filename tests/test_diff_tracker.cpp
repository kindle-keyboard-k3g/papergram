#include "test_framework.h"
#include "../src/graphics/diff_tracker.h"
#include "../src/graphics/canvas.h"
#include <vector>

TEST(diff_tracker_identical_buffers_produce_no_changes) {
    Canvas c1;
    Canvas c2;

    BufferDiffTracker diff;
    diff.compare(c1.frontBuffer(), c2.backBuffer());

    ASSERT_FALSE(diff.hasChanges());
    ASSERT_FALSE(diff.hasDarkToWhite());
    ASSERT_EQ(0U, diff.changed().count());
    ASSERT_EQ(0U, diff.darkToWhite().count());
}

TEST(diff_tracker_detects_white_to_black_as_changed_only) {
    Canvas c1; // all white
    Canvas c2; // all white initially
    c2.fillRect(10, 20, 30, 40, GrayscaleColor::BLACK);

    BufferDiffTracker diff;
    diff.compare(c1.backBuffer(), c2.backBuffer());

    ASSERT_TRUE(diff.hasChanges());
    ASSERT_FALSE(diff.hasDarkToWhite());
    ASSERT_EQ(BoundingBox(10, 20, 30, 40), diff.changed().boundingBox());
}

TEST(diff_tracker_detects_black_to_white_in_both_changed_and_dark_to_white) {
    Canvas c1; // previous has black box
    c1.fillRect(50, 60, 80, 90, GrayscaleColor::BLACK);
    Canvas c2; // current is all white

    BufferDiffTracker diff;
    diff.compare(c1.backBuffer(), c2.backBuffer());

    ASSERT_TRUE(diff.hasChanges());
    ASSERT_TRUE(diff.hasDarkToWhite());
    ASSERT_EQ(BoundingBox(50, 60, 80, 90), diff.changed().boundingBox());
    ASSERT_EQ(BoundingBox(50, 60, 80, 90), diff.darkToWhite().boundingBox());
}

TEST(diff_tracker_detects_dark_gray_to_white_in_dark_to_white) {
    Canvas c1;
    c1.fillRect(100, 100, 120, 120, GrayscaleColor::DARK_GRAY);
    Canvas c2; // white

    BufferDiffTracker diff;
    diff.compare(c1.backBuffer(), c2.backBuffer());

    ASSERT_TRUE(diff.hasDarkToWhite());
    ASSERT_EQ(BoundingBox(100, 100, 120, 120), diff.darkToWhite().boundingBox());
}

TEST(diff_tracker_keeps_disjoint_regions_distinct) {
    Canvas c1;
    Canvas c2;
    c2.fillRect(10, 10, 20, 20, GrayscaleColor::BLACK);
    c2.fillRect(100, 100, 120, 120, GrayscaleColor::BLACK);

    BufferDiffTracker diff;
    diff.compare(c1.backBuffer(), c2.backBuffer());

    ASSERT_EQ(2U, diff.changed().count());
}

TEST(diff_tracker_caps_excessive_fragmentation_to_full_screen) {
    std::vector<std::uint8_t> prev(600 * 800, GrayscaleColor::WHITE.value());
    std::vector<std::uint8_t> curr = prev;

    // Create 70 disjoint small dots across lines
    for (int i = 0; i < 70; ++i) {
        curr[i * 10 * 600 + i * 5] = GrayscaleColor::BLACK.value();
    }

    BufferDiffTracker diff;
    diff.compare(prev.data(), curr.data(), 600, 800);

    ASSERT_EQ(1U, diff.changed().count());
    ASSERT_EQ(BoundingBox(0, 0, 599, 799), diff.changed().boundingBox());
}
