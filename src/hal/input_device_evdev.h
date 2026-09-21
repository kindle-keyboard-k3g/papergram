#ifndef KINDLE_HAL_INPUT_DEVICE_EVDEV_H
#define KINDLE_HAL_INPUT_DEVICE_EVDEV_H

#include "input_device.h"
#include <vector>

class InputDeviceEvdev : public IInputDevice {
public:
    InputDeviceEvdev();
    ~InputDeviceEvdev() override;

    bool pollEvent(InputEvent& out_event, int timeout_ms) override;

private:
    std::vector<int> fds_;
    bool has_devices_ = false;

    void openDevices();
    bool translateCode(std::uint16_t type, std::uint16_t code, std::int32_t value, InputEvent& out) const;
};

#endif
