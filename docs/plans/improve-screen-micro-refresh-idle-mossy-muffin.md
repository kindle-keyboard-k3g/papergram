# Implementation Plan: Screen Micro-Refresh, Anti-Ghosting, and Multi-Pass Cleaning

## Context
Kindle 3 (Kindle Keyboard / e-ink Pearl 600x800) displays suffer from ghosting when using fast partial updates (`fx_update_partial` / DU mode). The user requested three specific enhancements:
1. **Idle Micro-Refresh:** When the client is idle, gradually sweep the entire screen in slices/tiles to clean residual ghosting artifacts without disrupting the user. The chosen configuration is **Interleaved / Grid tiles** (checkerboard-ordered grid) with a **Slow cadence** (~5 seconds idle before starting, 400ms per tile step).
2. **Anti-Ghosting Techniques:** Enforce screen transition full refreshes, restore periodic GC16 full flashes based on keystroke counts (restoring the existing `FullRefresh` strategy logic currently bypassed in `main.cpp`), and add an explicit full-refresh shortcut (`Alt+G` / Ghostbuster).
3. **Multi-Pass Cleaning (Black-to-White Transitions):** When a region of the screen changes from dark/black to white (e.g. an inverted cursor, dialog box dismissing, list selection moving), e-ink particles often leave noticeable faint shadows. We will perform a buffer comparison (`frontBuffer` vs `backBuffer`) and apply a dedicated second DU cleaning pass exclusively over areas that transitioned from dark to white.

---

## Architectural Changes & Key Components

### 1. `FrameDiffTracker` (`src/graphics/frame_diff_tracker.h/.cpp`)
- **Responsibility:** Compares `Canvas::frontBuffer()` (previous frame) with `Canvas::backBuffer()` (current frame) across 600x800 pixels.
- Generates:
  - `DirtyTracker dirty_all`: Bounding box(es) of all changed pixels between front and back buffers.
  - `DirtyTracker dark_to_white`: Bounding box(es) of regions where pixels transitioned from non-white (`GrayscaleColor::BLACK`, `DARK_GRAY`, or `LIGHT_GRAY`) to `GrayscaleColor::WHITE`.
- Keeps entities focused and adheres to Object Calisthenics (<= 15 lines/method, <= 2 member vars).

### 2. `IdleRefreshScheduler` (`src/graphics/idle_refresh_scheduler.h/.cpp`)
- **Responsibility:** Coordinates non-intrusive background cleaning during idle periods.
- Configuration:
  - Idle delay threshold: 5000 ms (5 seconds).
  - Step interval: 400 ms.
  - Grid geometry: 4 columns x 4 rows = 16 tiles of 150x200 pixels each.
  - Slicing order: Interleaved / Checkerboard order (e.g., alternating tiles: `(0,0), (2,0), (1,1), (3,1)...` then remaining tiles) to distribute visual refresh unobtrusively.
- Lifecycle:
  - `noteActivity()`: Resets idle timer and stops/resets current sweep.
  - `tick(uint64_t current_time_ms)`: Checks if idle threshold has elapsed. If ready and interval reached, issues a single DU `updateArea` for the next tile. Once all 16 tiles are swept, idles until next idle period.
  - Immediate preemption: Any user input instantly aborts the sweep.

### 3. `RefreshCoordinator` (`src/graphics/refresh_coordinator.h/.cpp`)
- **Responsibility:** Unifies the refresh workflow into a clean, testable boundary:
  - Invokes `FrameDiffTracker` on `canvas`.
  - Determines if screen changed or keystroke count exceeded limit -> triggers GC16 `fullRefresh()`.
  - Otherwise, issues DU `updateArea()` for changed regions.
  - If any regions transitioned from dark to white, issues a 2nd DU pass over those specific boxes to clean ghosting.
  - Calls `canvas.swapBuffers()`.
  - Integrates `IdleRefreshScheduler`.

### 4. Input & Keycode Additions (`src/hal/input_device.h`, `input_device_evdev.cpp`, `fallback_devices.cpp`)
- Add `KeyCode::KEY_ALT` to support `Alt+G` ghostbuster trigger.
- Track Alt state; when `Alt + G` is pressed, trigger an immediate manual GC16 full refresh.

### 5. Main Event Loop Update (`src/main.cpp`)
- Replace the current hardcoded `dirty_tracker.mark(0,0,599,799)` + `typing_refresh` loop with `RefreshCoordinator`.
- Fix the potential busy-spin in `InputDeviceEvdev::pollEvent` when no devices are open.
- Poll with small timeout (e.g. 50ms-100ms) and invoke `idle_scheduler.tick()` on timeout.

---

## Critical Files to Modify / Create

- `src/graphics/frame_diff_tracker.h` / `.cpp` (New)
- `src/graphics/idle_refresh_scheduler.h` / `.cpp` (New)
- `src/graphics/refresh_coordinator.h` / `.cpp` (New)
- `src/hal/input_device.h` (Add `KEY_ALT`)
- `src/hal/input_device_evdev.cpp` (Map `KEY_LEFTALT` / `KEY_RIGHTALT`)
- `src/hal/fallback_devices.cpp` (Support Alt key in terminal fallback if applicable)
- `src/main.cpp` (Integrate coordinator, idle ticks, screen transition detection, Alt+G)
- `tests/test_frame_diff_tracker.cpp` (New unit tests)
- `tests/test_idle_refresh_scheduler.cpp` (New unit tests)
- `tests/test_refresh_coordinator.cpp` (New unit tests)
- `tests/test_eink_strategy.cpp` (Update/extend existing tests)

---

## Verification Plan

1. **Unit Tests:**
   - `make test`: Build and run all existing + new unit tests verifying:
     - `FrameDiffTracker`: Correct detection of changed bounding boxes; accurate identification of black-to-white transitions.
     - `IdleRefreshScheduler`: Verifies 5s idle threshold, 400ms tick interval, interleaved tile ordering, and preemption upon `noteActivity()`.
     - `RefreshCoordinator`: Verifies 2-pass DU update on black-to-white transitions, GC16 full refresh on screen transition, and GC16 every 15 keystrokes.
2. **ASan & UBSan Tests:**
   - `make test-asan`: Ensure zero memory leaks or undefined behavior with new buffer comparisons and grid algorithms.
3. **Host Simulation:**
   - Run `./bin/kindle-telegram` in host mode (`/tmp/kindle_fb.ppm` output) to ensure smooth input response, clean idle ticking, and no CPU spinning.
