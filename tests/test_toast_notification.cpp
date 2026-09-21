#include "test_framework.h"

#include "../src/graphics/canvas.h"
#include "../src/ui/toast_notification.h"

#include <string>
#include <vector>

TEST(toast_notification_keeps_bounds_and_renders_card_content) {
    const BoundingBox bounds(100, 100, 499, 220);
    const std::vector<std::string> lines = {"First line", "Second line"};
    ui::ToastNotification notification(bounds, "Notice", lines);
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);

    notification.render(canvas);

    ASSERT_EQ(bounds, notification.bounds());
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(100, 100, 499, 220)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(112, 108, 180, 123)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(112, 130, 220, 145)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(112, 148, 220, 163)));
    ASSERT_EQ(GrayscaleColor::BLACK,
              canvas.pixelAt(ScreenCoordinate(114, 109)));
    ASSERT_EQ(GrayscaleColor::BLACK,
              canvas.pixelAt(ScreenCoordinate(114, 131)));
    ASSERT_EQ(GrayscaleColor::BLACK,
              canvas.pixelAt(ScreenCoordinate(115, 149)));
}

TEST(toast_notification_clips_long_text_to_card_bounds) {
    const BoundingBox bounds(100, 100, 220, 180);
    const std::string long_text(80U, 'X');
    const std::vector<std::string> lines = {long_text, long_text};
    ui::ToastNotification notification(bounds, long_text, lines);
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);

    notification.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(100, 100, 220, 180)));
    ASSERT_FALSE(canvas.hasNonWhitePixel(BoundingBox(221, 100, 599, 180)));
    ASSERT_FALSE(canvas.hasNonWhitePixel(BoundingBox(100, 181, 220, 799)));
}

TEST(toast_notification_handles_small_bounds_without_overflow) {
    const BoundingBox bounds(0, 0, 16, 18);
    const std::vector<std::string> lines = {std::string(80U, 'Y')};
    ui::ToastNotification notification(bounds, std::string(80U, 'Z'), lines);
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);

    notification.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(0, 0, 16, 18)));
    ASSERT_FALSE(canvas.hasNonWhitePixel(BoundingBox(17, 0, 40, 30)));
}

TEST(toast_notification_handles_right_and_bottom_canvas_edges) {
    const BoundingBox bounds(592, 780, 600, 800);
    const std::vector<std::string> lines = {std::string(80U, 'Q')};
    ui::ToastNotification notification(bounds, std::string(80U, 'R'), lines);
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);

    notification.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(592, 780, 599, 799)));
}
