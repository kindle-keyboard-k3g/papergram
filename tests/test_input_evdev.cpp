#include "test_framework.h"
#include "../src/hal/input_device_evdev.h"
#include <linux/input.h>

#undef KEY_POWER

TEST(input_device_evdev_translates_power_keys) {
    InputDeviceEvdev device;
    InputEvent ev{};

    ASSERT_TRUE(device.translateCode(EV_KEY, 116, 1, ev));
    ASSERT_TRUE(ev.pressed);
    ASSERT_TRUE(ev.code == KeyCode::KEY_POWER);

    ASSERT_TRUE(device.translateCode(EV_KEY, 142, 1, ev));
    ASSERT_TRUE(ev.pressed);
    ASSERT_TRUE(ev.code == KeyCode::KEY_POWER);

    ASSERT_TRUE(device.translateCode(EV_KEY, 205, 0, ev));
    ASSERT_FALSE(ev.pressed);
    ASSERT_TRUE(ev.code == KeyCode::KEY_POWER);
}

TEST(input_device_evdev_translates_alt_keys) {
    InputDeviceEvdev device;
    InputEvent ev{};

    // KEY_LEFTALT is 56, KEY_RIGHTALT is 100 in linux/input.h
    ASSERT_TRUE(device.translateCode(EV_KEY, 56, 1, ev));
    ASSERT_TRUE(ev.pressed);
    ASSERT_TRUE(ev.code == KeyCode::KEY_ALT);

    ASSERT_TRUE(device.translateCode(EV_KEY, 100, 0, ev));
    ASSERT_FALSE(ev.pressed);
    ASSERT_TRUE(ev.code == KeyCode::KEY_ALT);
}
