#include "test_framework.h"
#include "../src/hal/fallback_devices.h"

TEST(memory_frame_buffer_stores_and_reads_pixels) {
    MemoryFrameBuffer fb;
    ASSERT_EQ(600U, fb.width());
    ASSERT_EQ(800U, fb.height());

    fb.clear(GrayscaleColor::WHITE);
    ASSERT_EQ(0xFF, fb.getPixel(ScreenCoordinate(50, 50)).value());

    fb.setPixel(ScreenCoordinate(50, 50), GrayscaleColor::BLACK);
    ASSERT_EQ(0x00, fb.getPixel(ScreenCoordinate(50, 50)).value());
}

TEST(dummy_eink_controller_accepts_updates) {
    DummyEinkController eink;
    eink.updateArea(BoundingBox(10, 10, 100, 100), false);
    eink.updateArea(BoundingBox(0, 0, 600, 800), true);
    eink.fullRefresh();
    ASSERT_TRUE(true);
}
