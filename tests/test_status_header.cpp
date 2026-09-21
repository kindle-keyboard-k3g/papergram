#include "test_framework.h"
#include "../src/graphics/canvas.h"
#include "../src/ui/status_header.h"

TEST(status_header_renders_title_and_battery_to_canvas) {
    StatusHeader header("Kindle Telegram");
    header.setNetworkStatus("[3G]");
    header.setBatteryLevel(85);

    Canvas canvas;
    header.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(0, 0, 599, 24)));
}
