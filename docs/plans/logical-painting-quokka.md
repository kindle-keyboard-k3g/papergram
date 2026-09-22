# Exit Button & Confirmation Dialog Implementation Plan

## Context
In Kindle Papergram (`/home/samuelcaldas/repos/kindle/papergram`), users need a way to cleanly exit the application from the e-ink interface. Currently, the menu contains an "Exit Papergram" menu item in `ScreenNavigator::populateMenu()`, but:
1. Selecting "Exit Papergram" executes `state_.exit_cb`, which is never bound in `main.cpp`.
2. There is no confirmation dialog to prevent accidental exits.
3. The user selected the following preferences:
   - **Placement**: Settings / Menu option (inside Kindle Menu).
   - **Action**: Prompt confirmation before exit.

This plan adds a reusable, modal `ConfirmationDialog` component adhering strictly to Object Calisthenics and Clean Code guidelines, integrates it into `ScreenNavigator` when "Exit Papergram" is selected, binds the exit callback in `main.cpp` to stop the event loop cleanly, and covers everything with unit tests.

---

## Architectural Design

### 1. Reusable `ConfirmationDialog` (`src/ui/confirmation_dialog.h` / `.cpp`)
A modal dialog overlay designed specifically for Kindle e-ink display:
- **Dimensions & Position**:
  - Modal card centered horizontally and vertically (e.g. 440px wide, 180px tall).
  - High-contrast border (double border pattern matching `KindleMenu`).
- **Visuals**:
  - Title bar ("=== CONFIRMATION ===").
  - Message prompt ("Are you sure you want to exit?").
  - Two buttons: `[ Cancel ]` and `[ Exit ]` (or `[ No ]` and `[ Yes ]`).
  - Active selection highlighted using inverted video (`canvas.invertRect(...)` or filled black box with white text).
- **Navigation / Input Handling**:
  - Initial selection: `Cancel` (safe default).
  - `KEY_LEFT` / `KEY_RIGHT` / `KEY_UP` / `KEY_DOWN`: Toggles between `Cancel` and `Confirm`.
  - `KEY_ENTER`: Executes the callback corresponding to the selected option and closes dialog.
  - `KEY_BACK` / `KEY_MENU`: Cancels and closes dialog.
  - Consumes all input while open (modal behavior).
- **Object Calisthenics Compliance**:
  - Class <= 100 lines, methods <= 15 lines.
  - Max 2 instance variables (e.g., `DialogState state_` struct wrapping fields).
  - Zero `else` keywords (early returns and guard clauses).
  - 1 indentation level per method.

### 2. Integration in `ScreenNavigator` (`src/ui/screen_navigator.h` / `.cpp`)
- Add `ui::ConfirmationDialog confirm_dialog_` (or state) inside `ScreenNavigator`.
- Update `ScreenNavigator::populateMenu()`:
  - When "Exit Papergram" is selected from `KindleMenu`:
    - Menu closes.
    - `confirm_dialog_.open(...)` is triggered with:
      - Prompt: "Exit Papergram?"
      - Confirm action: `[this]() { if (state_.exit_cb) state_.exit_cb(); }`
      - Cancel action: `[this]() { /* dialog closes, returns to screen */ }`
- Update `ScreenNavigator::handleInput()`:
  - Priority dispatch:
    1. If `confirm_dialog_.isOpen()`: route input to `confirm_dialog_.handleInput(event)`.
    2. Else if `menu_.isOpen()`: route input to `menu_.handleInput(event)`.
    3. Else: route input to active screen.
- Update `ScreenNavigator::render()`:
  - Render active screen.
  - If `menu_.isOpen()`, render menu.
  - If `confirm_dialog_.isOpen()`, render confirmation dialog on top.

### 3. Wire-up in `main.cpp` (`src/main.cpp`)
- Wire the exit callback to signal termination:
  ```cpp
  navigator.setExitCallback([&]() {
      g_running = 0;
  });
  ```
- When confirmed, `g_running = 0`, the main event loop terminates gracefully, printing:
  `[Kindle Telegram] Shutting down cleanly.`

---

## Critical Files to Create / Modify

1. **`src/ui/confirmation_dialog.h`** & **`src/ui/confirmation_dialog.cpp`** (Create)
   - Implement `ConfirmationDialog` with open/close, selection toggle, enter confirmation, and render logic.
2. **`src/ui/screen_navigator.h`** & **`src/ui/screen_navigator.cpp`** (Modify)
   - Add `ConfirmationDialog confirm_dialog_` member.
   - Wire "Exit Papergram" menu action to open the dialog.
   - Route input and render to the dialog when open.
3. **`src/main.cpp`** (Modify)
   - Call `navigator.setExitCallback([&]() { g_running = 0; });`.
4. **`tests/test_confirmation_dialog.cpp`** (Create)
   - Test dialog initial closed state.
   - Test open, close, and cancel via `KEY_BACK`.
   - Test selection toggle with arrow keys.
   - Test execution of confirm vs cancel callbacks on `KEY_ENTER`.
   - Test rendering to canvas.
5. **`tests/test_screen_navigator_menu.cpp`** (Modify)
   - Test selecting "Exit Papergram" opens confirmation dialog.
   - Test confirming exit invokes `exit_cb`.
   - Test cancelling exit closes dialog without invoking `exit_cb`.

---

## Verification & Testing Plan

1. **Unit Tests**:
   - Run `make test` in `/home/samuelcaldas/repos/kindle/papergram`:
     - Verify all existing 108 tests pass.
     - Verify all new `test_confirmation_dialog` and updated `test_screen_navigator_menu` tests pass.
2. **Sanitizer Checks**:
   - Run `make test-asan` to ensure 0 memory leaks and clean bounds checking.
3. **Host Build**:
   - Run `make client` to verify native compilation.
4. **ARM32 Cross-compilation**:
   - Run `make kindle-debug` or verify ARM toolchain build cleanly compiles.
