#ifndef KINDLE_UI_CONFIRMATION_DIALOG_H
#define KINDLE_UI_CONFIRMATION_DIALOG_H

#include "../graphics/canvas.h"
#include "../hal/input_device.h"
#include <functional>
#include <string>

namespace ui {

/** @brief Identifies the currently selected confirmation-dialog button. */
enum class DialogOption {
    /** @brief The cancel button is selected. */
    CANCEL,
    /** @brief The confirm button is selected. */
    CONFIRM
};

/**
 * @brief Presents a two-option modal dialog and invokes the chosen callback.
 */
class ConfirmationDialog {
public:
    /** @brief Creates a closed confirmation dialog. */
    ConfirmationDialog();

    /**
     * @brief Opens the dialog with labels, message, and callbacks.
     * @param title Dialog heading.
     * @param message Message displayed below the heading.
     * @param on_confirm Callback invoked when confirmation is selected.
     * @param on_cancel Optional callback invoked when cancellation is selected.
     * @param confirm_label Label displayed on the confirm button.
     * @param cancel_label Label displayed on the cancel button.
     */
    void open(std::string title,
              std::string message,
              std::function<void()> on_confirm,
              std::function<void()> on_cancel = nullptr,
              std::string confirm_label = "Exit",
              std::string cancel_label = "Cancel");

    /** @brief Closes the dialog without invoking a callback. */
    void close();

    /**
     * @brief Reports whether the dialog is visible.
     * @return True when the dialog is open.
     */
    bool isOpen() const;

    /**
     * @brief Returns the currently selected option.
     * @return Selected cancel or confirm option.
     */
    DialogOption selectedOption() const;

    /**
     * @brief Handles dismissal, selection, and confirmation input.
     * @param event Input event to process.
     * @return True when the dialog consumes the event.
     */
    bool handleInput(const InputEvent& event);

    /**
     * @brief Renders the dialog when it is open.
     * @param canvas Destination canvas.
     */
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
