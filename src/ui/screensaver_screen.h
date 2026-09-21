#ifndef KINDLE_UI_SCREENSAVER_SCREEN_H
#define KINDLE_UI_SCREENSAVER_SCREEN_H

#include "screen.h"

class ScreenNavigator;

class ScreensaverScreen : public IScreen {
public:
    explicit ScreensaverScreen(ScreenNavigator* navigator = nullptr);

    void render(Canvas& canvas) override;
    void handleInput(const InputEvent& event) override;
    void onEnter() override;
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
