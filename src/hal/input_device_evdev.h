#ifndef KINDLE_HAL_INPUT_DEVICE_EVDEV_H
#define KINDLE_HAL_INPUT_DEVICE_EVDEV_H

#include "input_device.h"
#include <vector>

/**
 * @brief Linux evdev input device multiplexer.
 *
 * Opens the Kindle keyboard, directional pad, and page-button event devices
 * and translates their Linux key codes into application-level KeyCode values.
 */
class InputDeviceEvdev : public IInputDevice {
public:
    /**
     * @brief Opens the supported Linux evdev input devices.
     *
     * Missing devices are tolerated so the object can still be polled on a
     * host without Kindle input hardware.
     */
    InputDeviceEvdev();

    /** @brief Closes all evdev descriptors opened by this device. */
    ~InputDeviceEvdev() override;

    /**
     * @brief Waits for and translates one Linux input event.
     * @param out_event Destination for the normalized event.
     * @param timeout_ms Maximum wait time in milliseconds.
     * @return true when a supported event was translated; otherwise false.
     */
    bool pollEvent(InputEvent& out_event, int timeout_ms) override;

    /**
     * @brief Translates a Linux evdev event into an application event.
     * @param type Linux input event type.
     * @param code Linux input event code.
     * @param value Linux key value, where zero represents release.
     * @param out Destination for the normalized event.
     * @return true when the event is a supported key event; otherwise false.
     */
    bool translateCode(std::uint16_t type, std::uint16_t code,
                       std::int32_t value, InputEvent& out) const;

private:
    std::vector<int> fds_;
    bool has_devices_ = false;

    void openDevices();
};

#endif
