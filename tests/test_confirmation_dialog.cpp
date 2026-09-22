#include "test_framework.h"
#include "../src/ui/confirmation_dialog.h"
#include "../src/graphics/canvas.h"

TEST(confirmation_dialog_initial_state_is_closed) {
    ui::ConfirmationDialog dialog;
    ASSERT_FALSE(dialog.isOpen());
}

TEST(confirmation_dialog_open_sets_state_and_defaults_to_cancel) {
    ui::ConfirmationDialog dialog;
    dialog.open("=== TITLE ===", "Prompt text", []() {});
    ASSERT_TRUE(dialog.isOpen());
    ASSERT_TRUE(dialog.selectedOption() == ui::DialogOption::CANCEL);
}

TEST(confirmation_dialog_close_resets_open_state) {
    ui::ConfirmationDialog dialog;
    dialog.open("=== TITLE ===", "Prompt text", []() {});
    ASSERT_TRUE(dialog.isOpen());
    dialog.close();
    ASSERT_FALSE(dialog.isOpen());
}

TEST(confirmation_dialog_arrow_keys_toggle_selection) {
    ui::ConfirmationDialog dialog;
    dialog.open("=== TITLE ===", "Prompt text", []() {});

    ASSERT_TRUE(dialog.selectedOption() == ui::DialogOption::CANCEL);

    InputEvent right_ev{KeyCode::KEY_RIGHT, true};
    ASSERT_TRUE(dialog.handleInput(right_ev));
    ASSERT_TRUE(dialog.selectedOption() == ui::DialogOption::CONFIRM);

    InputEvent left_ev{KeyCode::KEY_LEFT, true};
    ASSERT_TRUE(dialog.handleInput(left_ev));
    ASSERT_TRUE(dialog.selectedOption() == ui::DialogOption::CANCEL);

    InputEvent up_ev{KeyCode::KEY_UP, true};
    ASSERT_TRUE(dialog.handleInput(up_ev));
    ASSERT_TRUE(dialog.selectedOption() == ui::DialogOption::CONFIRM);

    InputEvent down_ev{KeyCode::KEY_DOWN, true};
    ASSERT_TRUE(dialog.handleInput(down_ev));
    ASSERT_TRUE(dialog.selectedOption() == ui::DialogOption::CANCEL);
}

TEST(confirmation_dialog_back_and_menu_keys_cancel) {
    ui::ConfirmationDialog dialog;
    bool cancelled = false;
    dialog.open("=== TITLE ===", "Prompt text", []() {}, [&cancelled]() {
        cancelled = true;
    });

    InputEvent back_ev{KeyCode::KEY_BACK, true};
    ASSERT_TRUE(dialog.handleInput(back_ev));
    ASSERT_FALSE(dialog.isOpen());
    ASSERT_TRUE(cancelled);

    cancelled = false;
    dialog.open("=== TITLE ===", "Prompt text", []() {}, [&cancelled]() {
        cancelled = true;
    });
    InputEvent menu_ev{KeyCode::KEY_MENU, true};
    ASSERT_TRUE(dialog.handleInput(menu_ev));
    ASSERT_FALSE(dialog.isOpen());
    ASSERT_TRUE(cancelled);
}

TEST(confirmation_dialog_enter_executes_selected_option) {
    ui::ConfirmationDialog dialog;
    bool confirmed = false;
    bool cancelled = false;

    // First test: default CANCEL selected
    dialog.open("=== TITLE ===", "Prompt text",
                [&confirmed]() { confirmed = true; },
                [&cancelled]() { cancelled = true; });

    InputEvent enter_ev{KeyCode::KEY_ENTER, true};
    ASSERT_TRUE(dialog.handleInput(enter_ev));
    ASSERT_FALSE(dialog.isOpen());
    ASSERT_FALSE(confirmed);
    ASSERT_TRUE(cancelled);

    // Second test: toggled to CONFIRM
    confirmed = false;
    cancelled = false;
    dialog.open("=== TITLE ===", "Prompt text",
                [&confirmed]() { confirmed = true; },
                [&cancelled]() { cancelled = true; });

    dialog.handleInput(InputEvent{KeyCode::KEY_RIGHT, true});
    ASSERT_TRUE(dialog.selectedOption() == ui::DialogOption::CONFIRM);

    ASSERT_TRUE(dialog.handleInput(enter_ev));
    ASSERT_FALSE(dialog.isOpen());
    ASSERT_TRUE(confirmed);
    ASSERT_FALSE(cancelled);
}

TEST(confirmation_dialog_swallows_other_keys) {
    ui::ConfirmationDialog dialog;
    // When closed, returns false
    ASSERT_FALSE(dialog.handleInput(InputEvent{KeyCode::KEY_A, true}));

    dialog.open("=== TITLE ===", "Prompt text", []() {});
    // When open, unhandled key returns true (swallowed) and selection unchanged
    ASSERT_TRUE(dialog.handleInput(InputEvent{KeyCode::KEY_A, true}));
    ASSERT_TRUE(dialog.isOpen());
    ASSERT_TRUE(dialog.selectedOption() == ui::DialogOption::CANCEL);

    // Key up event (pressed == false) returns false
    ASSERT_FALSE(dialog.handleInput(InputEvent{KeyCode::KEY_A, false}));
}

TEST(confirmation_dialog_renders_to_canvas) {
    Canvas canvas;
    ui::ConfirmationDialog dialog;
    dialog.open("=== EXIT PAPERGRAM ===", "Are you sure you want to exit?", []() {});
    dialog.render(canvas);

    // Toggle to confirm and render again
    dialog.handleInput(InputEvent{KeyCode::KEY_RIGHT, true});
    dialog.render(canvas);
}
