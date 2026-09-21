# Task 1 Report: `ToastNotification` UI Component

## Status

DONE

## Commit

`f8419a2` — `feat(ui): add ToastNotification component and unit tests`

## Implementation

- Added `/home/samuelcaldas/repos/kindle/papergram/src/ui/toast_notification.h` with the reusable `ui::ToastNotification` interface.
- Added `/home/samuelcaldas/repos/kindle/papergram/src/ui/toast_notification.cpp` with:
  - light-gray background fill;
  - black outer frame and dark-gray inner frame;
  - black title rendering;
  - black body-line rendering with 18-pixel line spacing;
  - bounds-aware body-line clipping.
- Added `/home/samuelcaldas/repos/kindle/papergram/tests/test_toast_notification.cpp` covering bounds retention, card-region rendering, title/body non-white pixels, and representative black text pixels.
- No test-runner or Makefile change was necessary because the existing wildcard rules automatically discover new test and UI source files.

## TDD and Test Results

1. Red phase: `make test` failed because `toast_notification.h` did not exist.
2. Green phase: `make test` passed with **39 passed, 0 failed**.
3. Sanitizer validation: `make test-asan` passed with **39 passed, 0 failed** and no sanitizer diagnostics.
4. Native build validation: `make client` completed successfully.
5. Formatting validation: `git diff --check` and staged diff checks completed without errors.

## Concerns

The task brief referenced `graphics::Canvas`, `graphics::BoundingBox`, and `graphics/types.h`, but this repository currently exposes these graphics types globally and has no `src/graphics/types.h`. The implementation follows the repository's existing API while keeping the requested `ui::ToastNotification` namespace and behavior.

## Fix Round 1

### Reviewer Findings Addressed

- Added width-aware truncation using the available toast and canvas width, preventing long titles and body lines from drawing outside the card.
- Added full-glyph vertical fit checks against both toast bounds and the 600x800 canvas before constructing `ScreenCoordinate` values.
- Added boundary tests for small bounds, long text, right-edge placement, and bottom-edge placement.

### Validation

- Red phase: the new tests reproduced horizontal overflow and an edge-position `std::out_of_range` failure before the renderer fix.
- `make test`: **42 passed, 0 failed**.
- `make test-asan`: **42 passed, 0 failed** with no sanitizer diagnostics.
- `git diff --check`: passed.

### Fix Commit

`4b66ac9` — `fix(ui): clip toast notification text safely`
