# Task 1 Brief: Add `PopupTypes` and Extend `ToastNotification`

## Goal
Implement domain/UI value types for popup notifications (`src/ui/popup_types.h` and `src/ui/popup_types.cpp`) and extend `ToastNotification` (`src/ui/toast_notification.h` and `src/ui/toast_notification.cpp`) with `setBounds` so cached cards can be repositioned dynamically without reallocations.

## Files
- Create: `src/ui/popup_types.h`
- Create: `src/ui/popup_types.cpp`
- Modify: `src/ui/toast_notification.h`
- Modify: `src/ui/toast_notification.cpp`
- Test: update `tests/test_toast_notification.cpp` or add unit tests for `PopupTypes`

## Requirements
1. `src/ui/popup_types.h`:
   - `enum class PopupPosition { BOTTOM, TOP };`
   - `enum class PopupMode { VISUAL_STACK, SEQUENTIAL_QUEUE };`
   - `class PopupDuration`: wraps `std::chrono::milliseconds`, default 4000ms, guards against <= 0, methods `toMillis() const`, `defaultValue()`.
   - `class PopupTimestamp`: wraps `std::chrono::milliseconds` monotonic timestamp, methods `millis() const`, operator overloads (`+`, `-`, `>=`, `<`).
   - Adhere to Object Calisthenics: single indentation level, 0 `else`, methods <= 15 lines, classes <= 100 lines, max 2 instance variables.
2. `src/ui/toast_notification.h/.cpp`:
   - Add method: `void setBounds(const BoundingBox& bounds);`
   - Updates `data_.bounds` so that subsequent calls to `render(canvas)` use the updated bounds.
3. Verify with tests:
   - Test `PopupDuration` rejects 0/negative durations.
   - Test `PopupTimestamp` arithmetic and comparison.
   - Test `ToastNotification::setBounds` updates rendered position.
   - Run `make test` and `make test-asan`.

## Steps
1. Add tests in `tests/test_toast_notification.cpp` testing `setBounds` and popup types.
2. Implement `src/ui/popup_types.h`, `src/ui/popup_types.cpp`.
3. Update `src/ui/toast_notification.h` and `src/ui/toast_notification.cpp`.
4. Run `make test` and `make test-asan`.
5. Commit:
   `git add src/ui/popup_types.h src/ui/popup_types.cpp src/ui/toast_notification.h src/ui/toast_notification.cpp tests/test_toast_notification.cpp`
   `git commit -m "feat(ui): add PopupTypes value objects and ToastNotification setBounds"`

## Report Contract
Write report to `.superpowers/sdd/popup-notification-plan/task-1-report.md`.
Return: Status (DONE / DONE_WITH_CONCERNS / BLOCKED), Commit hash, Test summary, Concerns.
Do NOT spawn subagents.
