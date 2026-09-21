#ifndef KINDLE_TEST_MOCK_INPUT_DEVICE_H
#define KINDLE_TEST_MOCK_INPUT_DEVICE_H

#include "../../src/hal/input_device.h"

#include <deque>

class MockInputDevice : public IInputDevice {
public:
    void pushEvent(const InputEvent& event) {
        events_.push_back(event);
    }

    bool pollEvent(InputEvent& outEvent, int timeoutMs) override {
        static_cast<void>(timeoutMs);
        if (events_.empty()) {
            return false;
        }
        outEvent = events_.front();
        events_.pop_front();
        return true;
    }

private:
    std::deque<InputEvent> events_;
};

#endif
