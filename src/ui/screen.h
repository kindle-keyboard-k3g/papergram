#ifndef KINDLE_UI_SCREEN_H
#define KINDLE_UI_SCREEN_H

#include "../graphics/canvas.h"
#include "../hal/input_device.h"

class IScreen {
public:
    virtual ~IScreen() = default;

    virtual void render(Canvas& canvas) = 0;
    virtual void handleInput(const InputEvent& event) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
};

#endif
