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
