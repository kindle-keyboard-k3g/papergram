# Task 6 Brief: Main Loop Timer-Driven Integration

## Goal
Integrate `PopupManager` into the event loop in `src/main.cpp` so that:
1. `PopupManager` is instantiated at application scope.
2. Monotonic elapsed time (`steady_clock::now() - start_time`) is computed on every iteration of `while (g_running)`.
3. `popup_manager.update(now)` is called every iteration (even when no user input event is pressed).
4. If `popup_manager.needsRedraw()` or user input occurred:
   - Redraw the screen: `navigator.render(canvas);` then `popup_manager.render(canvas);`
   - Copy to framebuffer, flush.
   - If popup-only damage: mark `popup_manager.damageArea()` in dirty tracker (or full screen if input/screen transition).
   - Trigger partial e-ink refresh (`typing_refresh.refresh(...)`).
   - Call `popup_manager.acknowledgeRedraw()`.

## Files
- Modify: `src/main.cpp`
- Verify with `make test`, `make test-asan`, `make client`, `make kindle-debug`

## Requirements
- Respect Object Calisthenics: helper functions <= 15 lines, single indent level, 0 `else`.
- Keep event loop clean and readable.

## Steps
1. Inspect `src/main.cpp`.
2. Add `PopupManager popup_manager;` and `auto start_time = std::chrono::steady_clock::now();`.
3. Update the `while (g_running)` loop to call `popup_manager.update(now)` on every loop tick.
4. If `popup_manager.needsRedraw()`:
   - Render `navigator.render(canvas)` and `popup_manager.render(canvas)`.
   - Update dirty tracker and trigger e-ink refresh.
   - Acknowledge redraw.
5. Verify build with `make client` and `make test`.
6. Commit:
   `git add src/main.cpp`
   `git commit -m "feat(main): integrate PopupManager into main event loop with timer-driven e-ink refresh"`
