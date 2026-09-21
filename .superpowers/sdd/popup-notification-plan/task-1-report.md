# Task 1 Report: Add `PopupTypes` and Extend `ToastNotification`

## Status
DONE

## Implementation

- Added `/home/samuelcaldas/repos/kindle/papergram/src/ui/popup_types.h` and `/home/samuelcaldas/repos/kindle/papergram/src/ui/popup_types.cpp`.
- Added `PopupPosition` and `PopupMode` enums.
- Added `ui::PopupDuration` with:
  - positive-duration validation;
  - 4000 ms default constructor and `defaultValue()` factory;
  - `toMillis()` access.
- Added `ui::PopupTimestamp` with chrono millisecond storage, millisecond access, duration arithmetic, timestamp subtraction, and timestamp comparisons.
- Added `ToastNotification::setBounds(const BoundingBox&)` and updated rendering behavior to use the new bounds.
- Added tests covering non-positive duration rejection, the default duration, timestamp arithmetic/comparison, and repositioned toast rendering.

## TDD Evidence

- Initial `make test` failed during compilation because `src/ui/popup_types.h` did not yet exist.
- Implemented the required APIs.
- Subsequent validation passed.

## Verification

- `make test`: 48 passed, 0 failed.
- `make test-asan`: 48 passed, 0 failed; no AddressSanitizer or UndefinedBehaviorSanitizer diagnostics.
- `git diff --check`: passed.

## Commit

The implementation was committed with message:

`feat(ui): add PopupTypes value objects and ToastNotification setBounds`
