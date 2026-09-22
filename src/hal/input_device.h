#ifndef KINDLE_HAL_INPUT_DEVICE_H
#define KINDLE_HAL_INPUT_DEVICE_H

#include <cstdint>

/**
 * @brief Device-independent key identifiers understood by the application.
 */
enum class KeyCode : std::uint16_t {
    /** @brief Alphabetic key A. */
    KEY_A,
    /** @brief Alphabetic key B. */
    KEY_B,
    /** @brief Alphabetic key C. */
    KEY_C,
    /** @brief Alphabetic key D. */
    KEY_D,
    /** @brief Alphabetic key E. */
    KEY_E,
    /** @brief Alphabetic key F. */
    KEY_F,
    /** @brief Alphabetic key G. */
    KEY_G,
    /** @brief Alphabetic key H. */
    KEY_H,
    /** @brief Alphabetic key I. */
    KEY_I,
    /** @brief Alphabetic key J. */
    KEY_J,
    /** @brief Alphabetic key K. */
    KEY_K,
    /** @brief Alphabetic key L. */
    KEY_L,
    /** @brief Alphabetic key M. */
    KEY_M,
    /** @brief Alphabetic key N. */
    KEY_N,
    /** @brief Alphabetic key O. */
    KEY_O,
    /** @brief Alphabetic key P. */
    KEY_P,
    /** @brief Alphabetic key Q. */
    KEY_Q,
    /** @brief Alphabetic key R. */
    KEY_R,
    /** @brief Alphabetic key S. */
    KEY_S,
    /** @brief Alphabetic key T. */
    KEY_T,
    /** @brief Alphabetic key U. */
    KEY_U,
    /** @brief Alphabetic key V. */
    KEY_V,
    /** @brief Alphabetic key W. */
    KEY_W,
    /** @brief Alphabetic key X. */
    KEY_X,
    /** @brief Alphabetic key Y. */
    KEY_Y,
    /** @brief Alphabetic key Z. */
    KEY_Z,
    /** @brief Number key 0. */
    KEY_0,
    /** @brief Number key 1. */
    KEY_1,
    /** @brief Number key 2. */
    KEY_2,
    /** @brief Number key 3. */
    KEY_3,
    /** @brief Number key 4. */
    KEY_4,
    /** @brief Number key 5. */
    KEY_5,
    /** @brief Number key 6. */
    KEY_6,
    /** @brief Number key 7. */
    KEY_7,
    /** @brief Number key 8. */
    KEY_8,
    /** @brief Number key 9. */
    KEY_9,
    /** @brief Enter or confirm key. */
    KEY_ENTER,
    /** @brief Backspace key. */
    KEY_BACKSPACE,
    /** @brief Space bar. */
    KEY_SPACE,
    /** @brief Up navigation key. */
    KEY_UP,
    /** @brief Down navigation key. */
    KEY_DOWN,
    /** @brief Left navigation key. */
    KEY_LEFT,
    /** @brief Right navigation key. */
    KEY_RIGHT,
    /** @brief Previous-page key or rocker. */
    KEY_PAGEUP,
    /** @brief Next-page key or rocker. */
    KEY_PAGEDOWN,
    /** @brief Back or escape key. */
    KEY_BACK,
    /** @brief Menu key. */
    KEY_MENU,
    /** @brief Symbol modifier key. */
    KEY_SYM,
    /** @brief Power key. */
    KEY_POWER,
    /** @brief Alt modifier key. */
    KEY_ALT
};

/**
 * @brief Normalized input event delivered to application code.
 */
struct InputEvent {
    /** @brief Device-independent key identifier. */
    KeyCode code;

    /** @brief Whether the key was pressed or released. */
    bool pressed;
};

/**
 * @brief Abstract source of normalized keyboard and navigation events.
 */
class IInputDevice {
public:
    /** @brief Releases the input device resources. */
    virtual ~IInputDevice() = default;

    /**
     * @brief Waits for and reads one input event.
     * @param outEvent Destination for the normalized event.
     * @param timeoutMs Maximum wait time in milliseconds.
     * @return true when an event was written to outEvent; false on timeout,
     *     unavailable input, or an unsupported event.
     */
    virtual bool pollEvent(InputEvent& outEvent, int timeoutMs) = 0;

    /**
     * @brief Reports whether the input source has reached end-of-input.
     * @return true when the device is closed; otherwise false.
     */
    virtual bool isClosed() const { return false; }
};

#endif
