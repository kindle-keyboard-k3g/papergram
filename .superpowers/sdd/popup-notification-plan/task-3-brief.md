# Task 3 Brief: Implement `PopupManager`

## Goal
Implement the high-level, independent `PopupManager` in `src/ui/popup_manager.h` and `src/ui/popup_manager.cpp`.

## Files
- Create: `src/ui/popup_manager.h`
- Create: `src/ui/popup_manager.cpp`
- Test: create or expand `tests/test_popup_manager.cpp`

## Requirements & Behavior
1. Interface:
```cpp
#pragma once

#include "popup_collection.h"
#include "popup_layout.h"
#include "popup_types.h"
#include "../domain/message.h"

#include <optional>
#include <string>
#include <vector>

namespace ui {

class PopupManager {
public:
    PopupManager();

    bool show(const std::string& title,
              const std::vector<std::string>& lines,
              PopupDuration duration = PopupDuration::defaultValue());

    bool notifyIncomingMessage(const Message& message,
                                const std::optional<ChatId>& active_chat_id,
                                bool muted = false);

    bool update(PopupTimestamp now);
    void render(Canvas& canvas);

    void setMode(PopupMode mode);
    void setPosition(PopupPosition position);

    PopupMode mode() const;
    PopupPosition position() const;
    std::size_t visibleCount() const;
    std::size_t pendingCount() const;

    bool needsRedraw() const;
    const std::optional<BoundingBox>& damageArea() const;
    void acknowledgeRedraw();

private:
    struct State {
        PopupCollection collection;
        PopupLayout layout;
        PopupPosition position{PopupPosition::BOTTOM};
        PopupMode mode{PopupMode::VISUAL_STACK};
        PopupTimestamp current_time{std::chrono::milliseconds(0)};
        bool needs_redraw{false};
        std::optional<BoundingBox> damage_area;
    };
    State state_;
};

} // namespace ui
```

2. Incoming Message Logic:
   - If `message.isOutgoing()`, return `false`.
   - If `active_chat_id.has_value() && message.chatId() == *active_chat_id`, return `false`.
   - If `muted`, return `false`.
   - Construct title: `message.sender()` (or e.g. "Message from " + sender).
   - Construct line: `message.text().value()` (preview).
   - Show popup and mark `needs_redraw = true`, compute damage area.
   - Return `true`.

3. Time & Redraw Lifecycle:
   - Default position: `BOTTOM`. Default mode: `VISUAL_STACK`.
   - `update(now)`:
     - `state_.current_time = now;`
     - Call `collection.update(now, state_.mode)`.
     - If collection visible state changed:
       - `state_.needs_redraw = true;`
       - `state_.damage_area = state_.layout.calculateDamageArea(state_.collection.visibleCount(state_.mode), state_.position);`
       - Return `true`.
     - Return `false`.
   - `render(canvas)`:
     - If `collection.visibleCount(mode) == 0`, return immediately.
     - Call `collection.render(canvas, state_.layout, state_.position, state_.mode);`
     - No heap allocations inside `render()`!
   - `acknowledgeRedraw()`:
     - `state_.needs_redraw = false;`

4. Strict Object Calisthenics:
   - 0 `else` keywords.
   - Max 1 indent level per method.
   - Methods <= 15 lines, classes <= 100 lines.
   - Max 2 instance variables per class.

## Steps
1. Write unit tests in `tests/test_popup_manager.cpp`.
2. Implement `src/ui/popup_manager.h` and `src/ui/popup_manager.cpp`.
3. Run `make test` and `make test-asan`.
4. Commit:
   `git add src/ui/popup_manager.* tests/test_popup_manager.cpp`
   `git commit -m "feat(ui): implement PopupManager with incoming message filtering and timer lifecycle"`

## Report Contract
Write report to `.superpowers/sdd/popup-notification-plan/task-3-report.md`.
Return status, commit, test summary, concerns.
Do NOT spawn subagents.
