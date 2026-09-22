#ifndef KINDLE_UI_SCREEN_H
#define KINDLE_UI_SCREEN_H

#include "../graphics/canvas.h"
#include "../hal/input_device.h"
#include "menu_item.h"
#include <vector>

/**
 * @brief Defines the lifecycle and input contract for an application screen.
 */
class IScreen {
public:
    /** @brief Destroys the screen implementation. */
    virtual ~IScreen() = default;

    /**
     * @brief Renders the screen into the supplied canvas.
     * @param canvas Destination canvas for the screen contents.
     */
    virtual void render(Canvas& canvas) = 0;

    /**
     * @brief Processes one input event received by the screen.
     * @param event Input event to handle.
     */
    virtual void handleInput(const InputEvent& event) = 0;

    /** @brief Notifies the screen that it became active. */
    virtual void onEnter() = 0;

    /** @brief Notifies the screen that it is no longer active. */
    virtual void onExit() = 0;

    /**
     * @brief Returns menu actions specific to the current screen.
     * @return Contextual menu items, empty when no screen-specific actions exist.
     */
    virtual std::vector<ui::MenuItem> contextualMenuItems() { return {}; }
};


#endif
