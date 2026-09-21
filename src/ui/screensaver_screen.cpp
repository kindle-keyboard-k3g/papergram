#include "screensaver_screen.h"
#include "screen_navigator.h"

ScreensaverScreen::ScreensaverScreen(ScreenNavigator* navigator)
    : navigator_(navigator) {}

void ScreensaverScreen::render(Canvas& canvas) {
    canvas.clear(GrayscaleColor::WHITE);
    renderBorders(canvas);
    renderEmblem(canvas);
    renderCenterText(canvas);
    renderFooter(canvas);
}

void ScreensaverScreen::renderBorders(Canvas& canvas) const {
    canvas.drawRect(16, 16, 583, 783, GrayscaleColor::BLACK);
    canvas.drawRect(20, 20, 579, 779, GrayscaleColor::BLACK);
    canvas.drawLine(20, 24, 579, 24, GrayscaleColor::BLACK);
    canvas.drawLine(20, 775, 579, 775, GrayscaleColor::BLACK);
}

void ScreensaverScreen::renderEmblem(Canvas& canvas) const {
    canvas.drawRect(100, 260, 499, 520, GrayscaleColor::BLACK);
    canvas.drawRect(104, 264, 495, 516, GrayscaleColor::BLACK);
    canvas.drawLine(150, 345, 450, 345, GrayscaleColor::BLACK);
    canvas.drawLine(180, 450, 420, 450, GrayscaleColor::BLACK);
}

void ScreensaverScreen::renderCenterText(Canvas& canvas) const {
    canvas.blitText(ScreenCoordinate(256, 310), "K I N D L E");
    canvas.blitText(ScreenCoordinate(236, 380), "Device is Locked");
    canvas.blitText(ScreenCoordinate(216, 410), "Dispositivo Bloqueado");
}

void ScreensaverScreen::renderFooter(Canvas& canvas) const {
    canvas.blitText(ScreenCoordinate(196, 720), "Slide power switch to wake");
    canvas.blitText(ScreenCoordinate(160, 742), "Deslize o interruptor para reativar");
}

void ScreensaverScreen::handleInput(const InputEvent& event) {
    if (!event.pressed) return;
    if (event.code != KeyCode::KEY_POWER) return;
    wakeDevice();
}

void ScreensaverScreen::wakeDevice() {
    if (!navigator_) return;
    navigator_->unlockScreen();
}

void ScreensaverScreen::onEnter() {}
void ScreensaverScreen::onExit() {}
