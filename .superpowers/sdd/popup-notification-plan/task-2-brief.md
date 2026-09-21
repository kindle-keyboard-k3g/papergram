# Task 2 Brief: Implement `PopupLayout` and `PopupCollection`

## Goal
Implement layout calculation for popup notifications (`src/ui/popup_layout.h` and `src/ui/popup_layout.cpp`) and fixed-capacity first-class collection for popups (`src/ui/popup_collection.h` and `src/ui/popup_collection.cpp`).

## Files
- Create: `src/ui/popup_entry.h` and `src/ui/popup_entry.cpp` (or header-only/combined)
- Create: `src/ui/popup_layout.h` and `src/ui/popup_layout.cpp`
- Create: `src/ui/popup_collection.h` and `src/ui/popup_collection.cpp`
- Test: create `tests/test_popup_collection.cpp` or add tests in a new test suite

## Requirements & Behavior
1. `PopupEntry`:
   - Holds:
     - `ToastNotification card;`
     - `PopupTimestamp created_at;`
     - `PopupDuration duration;`
     - `bool active;`
   - Methods:
     - `isExpired(PopupTimestamp now) const;` -> `now >= created_at + duration.toMillis()`
     - `activate(PopupTimestamp now);` -> sets `created_at = now; active = true;`
     - Accessors/behavior without violating Object Calisthenics.
2. `PopupLayout`:
   - Canvas dimensions: 600x800.
   - Screen width: 600. Margins: left 30, right 570 (width 540). Card height: 68px.
   - Stack offset: 48px.
   - `BOTTOM` position:
     - Newest item (index 0) anchored at bottom: `y_bottom = 760`, `y_top = 760 - 68 = 692`.
     - Older stacked items (index 1, 2...): offset upward by `index * 48px`.
   - `TOP` position:
     - Newest item (index 0) anchored below StatusHeader (y=24): `y_top = 36`, `y_bottom = 36 + 68 = 104`.
     - Older stacked items: offset downward by `index * 48px`.
   - Clamps all coordinates within `0..599` and `0..799`.
   - `BoundingBox calculateBounds(std::size_t index, PopupPosition position) const;`
   - `BoundingBox calculateDamageArea(std::size_t count, PopupPosition position) const;`
3. `PopupCollection`:
   - First-class collection wrapping fixed-capacity `std::array<std::optional<PopupEntry>, 4>`.
   - No heap allocations in render or update loops!
   - Capacity: 4 slots.
   - Operations:
     - `bool push(const PopupEntry& entry, PopupMode mode, PopupTimestamp now);`
       - In `VISUAL_STACK`: if full (count == 4), drop oldest (shift items or drop slot 3) and place newest at slot 0.
       - In `SEQUENTIAL_QUEUE`: newest added to next available slot; if slot 0 active, subsequent slots wait until promoted.
     - `bool update(PopupTimestamp now, PopupMode mode);`
       - Checks active item(s) for expiration.
       - In `SEQUENTIAL_QUEUE`: if slot 0 expires, remove it, shift remaining items down, and activate new slot 0 with `activate(now)`.
       - Returns `true` if visible state changed (item expired, item promoted).
     - `std::size_t visibleCount(PopupMode mode) const;` (in sequential: max 1; in stack: active count).
     - `void render(Canvas& canvas, const PopupLayout& layout, PopupPosition position, PopupMode mode);`
4. Object Calisthenics:
   - 0 `else` keywords.
   - Max 1 indent level per method.
   - Classes <= 100 lines, methods <= 15 lines.
   - Max 2 instance variables per class.

## Steps
1. Write failing tests.
2. Implement components.
3. Verify with `make test` and `make test-asan`.
4. Commit:
   `git add src/ui/popup_entry.* src/ui/popup_layout.* src/ui/popup_collection.* tests/test_popup_collection.cpp`
   `git commit -m "feat(ui): implement PopupLayout and fixed-capacity PopupCollection"`

## Report Contract
Write report to `.superpowers/sdd/popup-notification-plan/task-2-report.md`.
Return status, commits, test summary, concerns.
Do NOT spawn subagents.
