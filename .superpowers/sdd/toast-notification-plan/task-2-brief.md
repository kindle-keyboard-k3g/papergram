# Task 2 Brief: Integrate Toast Instructions into `LoginScreen`

## Goal
Integrate `ToastNotification` into `LoginScreen` (`src/ui/login_screen.h` and `src/ui/login_screen.cpp`) to display keyboard instructions during phone number input (`step == 0`) and verification code input (`step == 1`), and guard the prefilled `+` prefix against deletion on backspace.

## Files
- Modify: `src/ui/login_screen.h`
- Modify: `src/ui/login_screen.cpp`
- Create: `tests/test_login_screen.cpp`
- Modify: `tests/test_runner.cpp` (if needed to add test suite)

## Requirements & Behavior
1. When `state_.step == 0` ("Enter your phone number"):
   - Render a `ToastNotification` box (e.g. from `ScreenCoordinate(40, 270)` to `ScreenCoordinate(560, 420)`):
     - Title: `"KEYBOARD TIPS:"`
     - Line 1: `"- '+' is already included for country code."`
     - Line 2: `"- Hold [ALT] + top row (Q-P) to type 1-0."`
     - Line 3: `"- Example: [ALT]+5 [ALT]+5 for Brazil +55"`
2. When `state_.step == 1` ("Enter verification code"):
   - Toast transitions to instructions for the 5-digit code:
     - Title: `"CODE TIPS:"`
     - Line 1: `"- Hold [ALT] + top row (Q-P) for digits."`
     - Line 2: `"- Check your Telegram app on phone/PC."`
3. Backspace guard:
   - When in phone number step (`state_.step == 0`), backspace must not erase the prefilled `+` (buffer length must not drop below 1 if first char is `+`, or ensure buffer starts with `+`).
4. Unit tests in `tests/test_login_screen.cpp`:
   - Verify phone step renders toast on canvas (non-white pixels in toast area).
   - Verify backspace on phone step does not delete prefilled `+`.
   - Verify transition to step 1 renders step 1 instructions or buffer resets appropriately.
5. Strict Object Calisthenics:
   - 0 `else` keywords.
   - Max 1 indentation level per method.
   - Methods <= 15 lines, classes <= 100 lines.
   - At most 2 instance variables per class.

## Steps
1. Write failing unit tests in `tests/test_login_screen.cpp`.
2. Verify tests fail via `make test`.
3. Update `src/ui/login_screen.h` and `src/ui/login_screen.cpp`.
4. Run `make test` and verify all tests pass.
5. Commit:
   `git add src/ui/login_screen.h src/ui/login_screen.cpp tests/test_login_screen.cpp tests/test_runner.cpp`
   `git commit -m "feat(ui): display keyboard and country code toast instructions on login screen"`

## Report Contract
Write report to `.superpowers/sdd/toast-notification-plan/task-2-report.md`.
Return status, commits, test summary, concerns.
Do NOT spawn any subagents.
