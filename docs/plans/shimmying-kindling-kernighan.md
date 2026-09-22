# Implementation Plan: Exit Button & Confirmation Dialog

## Context
Kindle Papergram currently provides an "Exit Papergram" option in `ScreenNavigator::populateMenu()`, but:
1. Triggering "Exit Papergram" executes `state_.exit_cb`, which is never bound in `main.cpp`.
2. There is no confirmation dialog, risking accidental application termination on Kindle e-ink.
3. User preferences established:
   - **Placement**: Settings / Menu option (inside Kindle Menu).
   - **Action**: Prompt confirmation before exit.

This plan details the implementation of a reusable `ConfirmationDialog` modal component adhering strictly to Object Calisthenics and Clean Code guidelines, its integration into `ScreenNavigator` upon selecting "Exit Papergram", binding the exit callback in `main.cpp` to stop the event loop cleanly, and comprehensive unit tests.

---

## Architectural Design

### 1. `ConfirmationDialog` Component (`src/ui/confirmation_dialog.h` / `src/ui/confirmation_dialog.cpp`)
A modal dialog overlay designed for Kindle Keyboard e-ink display:
- **Dimensions & Bounds**:
  - Modal card centered horizontally and vertically (e.g., width 440px, height 180px).
  - Centered on 600x800 canvas: `left = (600 - 440) / 2 = 80`, `top = (800 - 180) / 2 = 310`.
- **Styling**:
  - Double border frame (outer black 1px, inner dark gray 1px) matching `KindleMenu` design.
  - Title banner ("=== CONFIRMATION ===").
  - Message prompt ("Are you sure you want to exit?").
  - Two action buttons: `[ Cancel ]` and `[ Exit ]`.
  - Active button highlighted with inverted contrast (filled black box with white text).
- **Navigation & Input**:
  - Default selection: `DialogOption::CANCEL` (safe default).
  - `KEY_LEFT` / `KEY_RIGHT` / `KEY_UP` / `KEY_DOWN`: Toggles selection between `CANCEL` and `CONFIRM`.
  - `KEY_ENTER`: Executes the selected action callback and closes the dialog.
  - `KEY_BACK` / `KEY_MENU`: Cancels and closes the dialog.
  - Swallows all other key inputs when open (modal behavior).
- **Object Calisthenics Compliance**:
  - Class <= 100 lines, methods <= 15 lines.
  - <= 2 instance variables (state encapsulated in a `DialogState` struct).
  - Zero `else` keywords (guard clauses and early returns).
  - 1 indentation level per method.

### 2. `ScreenNavigator` Integration (`src/ui/screen_navigator.h` / `src/ui/screen_navigator.cpp`)
- Add `ui::ConfirmationDialog confirm_dialog_` member to `ScreenNavigator`.
- Update `populateMenu()`:
  - Selecting "Exit Papergram" closes the menu and opens `confirm_dialog_`:
    - Title: "=== EXIT PAPERGRAM ==="
    - Prompt: "Are you sure you want to exit?"
    - Confirm callback: `[this]() { if (state_.exit_cb) state_.exit_cb(); }`
    - Cancel callback: no-op / dialog closes.
- Update `handleInput(const InputEvent& event)`:
  - Check `confirm_dialog_.isOpen()` first; if open, delegate to `confirm_dialog_.handleInput(event)`.
  - Else if `menu_.isOpen()`, delegate to `menu_.handleInput(event)`.
  - Else delegate to active screen.
- Update `render(Canvas& canvas)`:
  - Render active screen.
  - If `menu_.isOpen()`, render menu.
  - If `confirm_dialog_.isOpen()`, render `confirm_dialog_` on top.
- Add `isConfirmDialogOpen() const` method for inspection and testing.

### 3. Application Lifecycle in `main.cpp` (`src/main.cpp`)
- Bind navigator exit callback:
  ```cpp
  navigator.setExitCallback([&]() {
      g_running = 0;
  });
  ```
- When confirmed, `g_running` becomes `0`, the main event loop terminates cleanly, and worker threads are joined.

---

## Critical Files to Create / Modify

1. **`src/ui/confirmation_dialog.h`** (Create)
   - Interface for `ConfirmationDialog`, state struct, open/close methods, input handling, and rendering.
2. **`src/ui/confirmation_dialog.cpp`** (Create)
   - Implementation conforming to Object Calisthenics and e-ink aesthetics.
3. **`src/ui/screen_navigator.h`** & **`src/ui/screen_navigator.cpp`** (Modify)
   - Embed `ConfirmationDialog confirm_dialog_`.
   - Update `populateMenu()`, `handleInput()`, `render()`, and add `isConfirmDialogOpen()`.
4. **`src/main.cpp`** (Modify)
   - Bind `navigator.setExitCallback([&]() { g_running = 0; });`.
5. **`tests/test_confirmation_dialog.cpp`** (Create)
   - Unit tests covering initial state, open/close, toggle selection, callback execution, and cancel keys.
6. **`tests/test_screen_navigator_menu.cpp`** (Modify)
   - Unit tests covering "Exit Papergram" menu selection opening confirmation dialog, confirmation triggering exit callback, and cancel dismissing dialog.

---

## Verification Plan

1. **Run Unit Tests**:
   - `make test` — verify all unit tests (existing 108 + new confirmation dialog tests) pass cleanly.
2. **Sanitizer Build & Test**:
   - `make test-asan` — verify zero leaks and address safety.
3. **Native Host Client Build**:
   - `make client` — build native executable and verify no compilation errors or warnings.
