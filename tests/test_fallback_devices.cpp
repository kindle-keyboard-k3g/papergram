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

TEST(stdin_input_device_initial_state_is_not_closed) {
    StdinInputDevice device;
    ASSERT_FALSE(device.isClosed());
}

TEST(stdin_input_device_maps_tilde_to_key_power) {
    StdinInputDevice device;
    InputEvent ev{};
    bool mapped = device.mapCharToEvent('~', ev);
    ASSERT_TRUE(mapped);
    ASSERT_TRUE(ev.pressed);
    ASSERT_TRUE(ev.code == KeyCode::KEY_POWER);
}

TEST(stdin_input_device_maps_m_to_key_menu) {
    StdinInputDevice device;
    InputEvent ev{};
    ASSERT_TRUE(device.mapCharToEvent('m', ev));
    ASSERT_TRUE(ev.pressed);
    ASSERT_TRUE(ev.code == KeyCode::KEY_MENU);

    ASSERT_TRUE(device.mapCharToEvent('M', ev));
    ASSERT_TRUE(ev.pressed);
    ASSERT_TRUE(ev.code == KeyCode::KEY_MENU);
}

