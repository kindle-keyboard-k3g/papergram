#ifndef KINDLE_UI_SCREENSAVER_SCREEN_H
#define KINDLE_UI_SCREENSAVER_SCREEN_H

#include "screen.h"

class ScreenNavigator;

/**
 * @brief Displays the locked-device screen and handles wake input.
 */
class ScreensaverScreen : public IScreen {
public:
    /**
     * @brief Creates a screensaver optionally connected to a navigator.
     * @param navigator Navigator used to unlock the device; may be null.
     */
    explicit ScreensaverScreen(ScreenNavigator* navigator = nullptr);

    /**
     * @brief Renders the locked-device artwork.
     * @param canvas Destination canvas for the screensaver.
     */
    void render(Canvas& canvas) override;

    /**
     * @brief Handles the power event that wakes the device.
     * @param event Input event to inspect.
     */
    void handleInput(const InputEvent& event) override;

    /** @brief Activates the screensaver screen. */
    void onEnter() override;

    /** @brief Deactivates the screensaver screen. */
    void onExit() override;

private:
    ScreenNavigator* navigator_;

    void renderBorders(Canvas& canvas) const;
    void renderEmblem(Canvas& canvas) const;
    void renderCenterText(Canvas& canvas) const;
    void renderFooter(Canvas& canvas) const;
    void wakeDevice();
};

#endif
