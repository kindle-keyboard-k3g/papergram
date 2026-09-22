#ifndef KINDLE_UI_CONFIRMATION_DIALOG_H
#define KINDLE_UI_CONFIRMATION_DIALOG_H

#include "../graphics/canvas.h"
#include "../hal/input_device.h"
#include <functional>
#include <string>

namespace ui {

enum class DialogOption {
    CANCEL,
    CONFIRM
};

class ConfirmationDialog {
public:
    ConfirmationDialog();

    void open(std::string title,
              std::string message,
              std::function<void()> on_confirm,
              std::function<void()> on_cancel = nullptr,
              std::string confirm_label = "Exit",
              std::string cancel_label = "Cancel");
    void close();
    bool isOpen() const;
    DialogOption selectedOption() const;

    bool handleInput(const InputEvent& event);
    void render(Canvas& canvas) const;

private:
    struct DialogState {
        std::string title;
        std::string message;
        std::string confirm_label{"Exit"};
        std::string cancel_label{"Cancel"};
        std::function<void()> on_confirm;
        std::function<void()> on_cancel;
        DialogOption selection{DialogOption::CANCEL};
        bool is_open{false};
    };

    DialogState state_;

    void toggleSelection();
    void executeSelection();
    void renderCard(Canvas& canvas, const BoundingBox& box) const;
    void renderHeader(Canvas& canvas, const BoundingBox& box) const;
    void renderMessage(Canvas& canvas, const BoundingBox& box) const;
    void renderButtons(Canvas& canvas, const BoundingBox& box) const;
    void renderButton(Canvas& canvas, const BoundingBox& box, const std::string& label, bool selected) const;
    static BoundingBox computeBounds();
};

} // namespace ui

#endif
