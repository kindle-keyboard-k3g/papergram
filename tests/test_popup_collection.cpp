#include "test_framework.h"

#include "../src/graphics/canvas.h"
#include "../src/ui/popup_collection.h"
#include "../src/ui/popup_entry.h"
#include "../src/ui/popup_layout.h"
#include "../src/ui/popup_types.h"
#include "../src/ui/toast_notification.h"

#include <chrono>
#include <string>
#include <vector>

namespace {

ui::PopupTimestamp timestamp(int milliseconds) {
    return ui::PopupTimestamp(std::chrono::milliseconds(milliseconds));
}

ui::PopupEntry entry(const std::string& title,
                     const std::string& line,
                     int duration = 1000) {
    const BoundingBox bounds(30, 0, 570, 67);
    const std::vector<std::string> lines = {line};
    const ui::ToastNotification card(bounds, title, lines);
    return ui::PopupEntry(card, ui::PopupDuration(std::chrono::milliseconds(duration)));
}

} // namespace

TEST(popup_layout_calculates_bottom_and_top_bounds) {
    const ui::PopupLayout layout;

    ASSERT_EQ(BoundingBox(30, 692, 570, 760),
              layout.calculateBounds(0U, PopupPosition::BOTTOM));
    ASSERT_EQ(BoundingBox(30, 644, 570, 712),
              layout.calculateBounds(1U, PopupPosition::BOTTOM));
    ASSERT_EQ(BoundingBox(30, 36, 570, 104),
              layout.calculateBounds(0U, PopupPosition::TOP));
    ASSERT_EQ(BoundingBox(30, 84, 570, 152),
              layout.calculateBounds(1U, PopupPosition::TOP));
}

TEST(popup_layout_clamps_bounds_and_damage_area) {
    const ui::PopupLayout layout;

    ASSERT_EQ(BoundingBox(30, 0, 570, 40),
              layout.calculateBounds(15U, PopupPosition::BOTTOM));
    ASSERT_EQ(BoundingBox(30, 548, 570, 760),
              layout.calculateDamageArea(4U, PopupPosition::BOTTOM));
    ASSERT_EQ(BoundingBox(30, 36, 570, 248),
              layout.calculateDamageArea(4U, PopupPosition::TOP));
}

TEST(popup_entry_activates_and_expires_at_duration_boundary) {
    ui::PopupEntry popup = entry("Title", "Message", 1000);
    ASSERT_FALSE(popup.isActive());

    popup.activate(timestamp(500));

    ASSERT_TRUE(popup.isActive());
    ASSERT_FALSE(popup.isExpired(timestamp(1499)));
    ASSERT_TRUE(popup.isExpired(timestamp(1500)));
}

TEST(popup_collection_visual_stack_keeps_newest_four_items) {
    ui::PopupCollection collection;
    const ui::PopupTimestamp now = timestamp(0);

    ASSERT_TRUE(collection.push(entry("One", "one"), PopupMode::VISUAL_STACK, now));
    ASSERT_TRUE(collection.push(entry("Two", "two"), PopupMode::VISUAL_STACK, now));
    ASSERT_TRUE(collection.push(entry("Three", "three"), PopupMode::VISUAL_STACK, now));
    ASSERT_TRUE(collection.push(entry("Four", "four"), PopupMode::VISUAL_STACK, now));
    ASSERT_TRUE(collection.push(entry("Five", "five"), PopupMode::VISUAL_STACK, now));

    ASSERT_EQ(static_cast<std::size_t>(4), collection.visibleCount(PopupMode::VISUAL_STACK));
    ASSERT_EQ(static_cast<std::size_t>(4), collection.size());

    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    collection.render(canvas, ui::PopupLayout(), PopupPosition::BOTTOM,
                      PopupMode::VISUAL_STACK);
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(30, 692, 570, 760)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(30, 548, 570, 616)));
}

TEST(popup_collection_sequential_queue_promotes_waiting_item) {
    ui::PopupCollection collection;
    const ui::PopupTimestamp start = timestamp(100);

    ASSERT_TRUE(collection.push(entry("First", "first", 1000),
                                PopupMode::SEQUENTIAL_QUEUE, start));
    ASSERT_TRUE(collection.push(entry("Second", "second", 1000),
                                PopupMode::SEQUENTIAL_QUEUE, start));

    ASSERT_EQ(static_cast<std::size_t>(1), collection.visibleCount(PopupMode::SEQUENTIAL_QUEUE));
    ASSERT_EQ(static_cast<std::size_t>(2), collection.size());
    ASSERT_FALSE(collection.update(timestamp(1099), PopupMode::SEQUENTIAL_QUEUE));
    ASSERT_TRUE(collection.update(timestamp(1100), PopupMode::SEQUENTIAL_QUEUE));
    ASSERT_EQ(static_cast<std::size_t>(1), collection.visibleCount(PopupMode::SEQUENTIAL_QUEUE));
    ASSERT_EQ(static_cast<std::size_t>(1), collection.size());

    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    collection.render(canvas, ui::PopupLayout(), PopupPosition::TOP,
                      PopupMode::SEQUENTIAL_QUEUE);
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(30, 36, 570, 104)));
}

TEST(popup_collection_sequential_queue_rejects_when_full) {
    ui::PopupCollection collection;
    const ui::PopupTimestamp now = timestamp(0);

    for (int index = 0; index < 4; ++index) {
        ASSERT_TRUE(collection.push(entry("Title", "Line"),
                                   PopupMode::SEQUENTIAL_QUEUE, now));
    }
    ASSERT_FALSE(collection.push(entry("Overflow", "Line"),
                                 PopupMode::SEQUENTIAL_QUEUE, now));
    ASSERT_EQ(static_cast<std::size_t>(4), collection.size());
}
