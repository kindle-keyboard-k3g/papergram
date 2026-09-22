#include "confirmation_dialog.h"
#include "../graphics/bitmap_font.h"

namespace {
constexpr int DIALOG_WIDTH = 440;
constexpr int DIALOG_HEIGHT = 180;
constexpr int DIALOG_LEFT = 80;
constexpr int DIALOG_TOP = 310;
constexpr int HEADER_HEIGHT = 34;
constexpr int BUTTON_WIDTH = 140;
constexpr int BUTTON_HEIGHT = 32;
constexpr int BUTTON_Y_OFFSET = 120;
constexpr int BUTTON_PADDING_X = 50;

int centeredTextX(int left, int width, std::size_t char_count) {
    int text_width = static_cast<int>(char_count) * 8;
    return left + (width - text_width) / 2;
}
} // namespace

namespace ui {

ConfirmationDialog::ConfirmationDialog() = default;

void ConfirmationDialog::open(std::string title,
                              std::string message,
                              std::function<void()> on_confirm,
                              std::function<void()> on_cancel,
                              std::string confirm_label,
                              std::string cancel_label) {
    state_.title = std::move(title);
    state_.message = std::move(message);
    state_.on_confirm = std::move(on_confirm);
    state_.on_cancel = std::move(on_cancel);
    state_.confirm_label = std::move(confirm_label);
    state_.cancel_label = std::move(cancel_label);
    state_.selection = DialogOption::CANCEL;
    state_.is_open = true;
}

void ConfirmationDialog::close() {
    state_.is_open = false;
}

bool ConfirmationDialog::isOpen() const {
    return state_.is_open;
}

DialogOption ConfirmationDialog::selectedOption() const {
    return state_.selection;
}

void ConfirmationDialog::toggleSelection() {
    if (state_.selection == DialogOption::CANCEL) {
        state_.selection = DialogOption::CONFIRM;
        return;
    }
    state_.selection = DialogOption::CANCEL;
}

void ConfirmationDialog::executeSelection() {
    auto on_confirm = state_.on_confirm;
    auto on_cancel = state_.on_cancel;
    DialogOption chosen = state_.selection;
    close();
    if (chosen == DialogOption::CONFIRM && on_confirm) {
        on_confirm();
        return;
    }
    if (chosen == DialogOption::CANCEL && on_cancel) {
        on_cancel();
    }
}

bool ConfirmationDialog::handleInput(const InputEvent& event) {
    if (!state_.is_open || !event.pressed) return false;
    if (event.code == KeyCode::KEY_BACK || event.code == KeyCode::KEY_MENU) {
        if (state_.on_cancel) state_.on_cancel();
        close();
        return true;
    }
    if (event.code == KeyCode::KEY_LEFT || event.code == KeyCode::KEY_RIGHT ||
        event.code == KeyCode::KEY_UP || event.code == KeyCode::KEY_DOWN) {
        toggleSelection();
        return true;
    }
    if (event.code == KeyCode::KEY_ENTER) {
        executeSelection();
        return true;
    }
    return true;
}

BoundingBox ConfirmationDialog::computeBounds() {
    return BoundingBox(DIALOG_LEFT, DIALOG_TOP,
                       DIALOG_LEFT + DIALOG_WIDTH, DIALOG_TOP + DIALOG_HEIGHT);
}

void ConfirmationDialog::render(Canvas& canvas) const {
    if (!state_.is_open) return;
    BoundingBox box = computeBounds();
    renderCard(canvas, box);
    renderHeader(canvas, box);
    renderMessage(canvas, box);
    renderButtons(canvas, box);
}

void ConfirmationDialog::renderCard(Canvas& canvas, const BoundingBox& box) const {
    canvas.fillRect(box, GrayscaleColor::WHITE);
    canvas.drawRect(box, GrayscaleColor::BLACK);
    canvas.drawRect(BoundingBox(box.left() + 2, box.top() + 2,
                                box.right() - 2, box.bottom() - 2),
                    GrayscaleColor::DARK_GRAY);
}

void ConfirmationDialog::renderHeader(Canvas& canvas, const BoundingBox& box) const {
    canvas.fillRect(BoundingBox(box.left() + 3, box.top() + 3,
                                box.right() - 3, box.top() + HEADER_HEIGHT),
                    GrayscaleColor::LIGHT_GRAY);
    int text_x = centeredTextX(box.left(), box.width(), state_.title.length());
    canvas.blitText(ScreenCoordinate(text_x, box.top() + 10),
                    state_.title, GrayscaleColor::BLACK);
    canvas.drawLine(ScreenCoordinate(box.left() + 3, box.top() + HEADER_HEIGHT),
                    ScreenCoordinate(box.right() - 3, box.top() + HEADER_HEIGHT),
                    GrayscaleColor::BLACK);
}

void ConfirmationDialog::renderMessage(Canvas& canvas, const BoundingBox& box) const {
    int text_x = centeredTextX(box.left(), box.width(), state_.message.length());
    canvas.blitText(ScreenCoordinate(text_x, box.top() + 65),
                    state_.message, GrayscaleColor::BLACK);
}

void ConfirmationDialog::renderButtons(Canvas& canvas, const BoundingBox& box) const {
    int cancel_left = box.left() + BUTTON_PADDING_X;
    int btn_top = box.top() + BUTTON_Y_OFFSET;
    BoundingBox cancel_box(cancel_left, btn_top,
                           cancel_left + BUTTON_WIDTH, btn_top + BUTTON_HEIGHT);
    bool cancel_sel = (state_.selection == DialogOption::CANCEL);
    renderButton(canvas, cancel_box, state_.cancel_label, cancel_sel);

    int confirm_left = box.right() - BUTTON_PADDING_X - BUTTON_WIDTH;
    BoundingBox confirm_box(confirm_left, btn_top,
                            confirm_left + BUTTON_WIDTH, btn_top + BUTTON_HEIGHT);
    bool confirm_sel = (state_.selection == DialogOption::CONFIRM);
    renderButton(canvas, confirm_box, state_.confirm_label, confirm_sel);
}

void ConfirmationDialog::renderButton(Canvas& canvas, const BoundingBox& box,
                                      const std::string& label, bool selected) const {
    if (selected) {
        canvas.fillRect(box, GrayscaleColor::BLACK);
        int text_x = centeredTextX(box.left(), box.width(), label.length());
        canvas.blitText(ScreenCoordinate(text_x, box.top() + 8),
                        label, GrayscaleColor::WHITE);
        return;
    }
    canvas.drawRect(box, GrayscaleColor::DARK_GRAY);
    int text_x = centeredTextX(box.left(), box.width(), label.length());
    canvas.blitText(ScreenCoordinate(text_x, box.top() + 8),
                    label, GrayscaleColor::BLACK);
}

} // namespace ui
