# Task 1 Brief: Create `ToastNotification` UI Component

## Goal
Implement a lightweight, reusable UI component `ToastNotification` in `src/ui/toast_notification.h` and `src/ui/toast_notification.cpp` that renders a styled notification container (rounded or double-lined border, filled background, multi-line text) using `Canvas`.

## Files
- Create: `src/ui/toast_notification.h`
- Create: `src/ui/toast_notification.cpp`
- Create: `tests/test_toast_notification.cpp`

## Interface
```cpp
#pragma once

#include "graphics/canvas.h"
#include "graphics/types.h"
#include <string>
#include <vector>

namespace ui {

class ToastNotification {
public:
    ToastNotification(const graphics::BoundingBox& bounds, const std::string& title, const std::vector<std::string>& lines);
    void render(graphics::Canvas& canvas) const;
    const graphics::BoundingBox& bounds() const;

private:
    struct ToastData {
        graphics::BoundingBox bounds;
        std::string title;
        std::vector<std::string> lines;
    };
    ToastData data_;
};

} // namespace ui
```

## Steps
1. Write failing unit test in `tests/test_toast_notification.cpp`:
   - Verify instantiation with bounds, title, and lines.
   - Verify `render(canvas)` draws non-white pixels within `bounds` for border, title, and body lines.
   - Use `tests/test_framework.h` macros (`TEST_CASE`, `ASSERT_TRUE`, `ASSERT_EQ`, etc.).
2. Run `make test` or compile test to verify failure.
3. Implement `src/ui/toast_notification.h` and `src/ui/toast_notification.cpp`:
   - Border / rectangle frame around `bounds` (can draw outer and inner borders or double lines with clear background / white fill).
   - Clear/fill background with white or light gray so it pops as a card/toast.
   - Title drawn at top inside bounds in `GrayscaleColor::BLACK`.
   - Body text lines blitted with proper line spacing (e.g. 18px line height).
   - Strict Object Calisthenics: single indentation level, 0 `else`, methods <= 15 lines, class <= 100 lines, at most 2 instance variables (uses `ToastData data_`).
4. Update `tests/test_runner.cpp` or Makefile if needed so `test_toast_notification.cpp` is linked and executed in `make test`.
5. Run `make test` and verify all tests pass.
6. Commit:
   `git add src/ui/toast_notification.h src/ui/toast_notification.cpp tests/test_toast_notification.cpp tests/test_runner.cpp`
   `git commit -m "feat(ui): add ToastNotification component and unit tests"`

## Report Contract
Write report to `.superpowers/sdd/toast-notification-plan/task-1-report.md`.
Return status (DONE, DONE_WITH_CONCERNS, NEEDS_CONTEXT, BLOCKED), commits, test summary, and any concerns.
Do NOT spawn any subagents.
