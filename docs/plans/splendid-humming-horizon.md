# Android-Style Popup & Notification System Plan

## Context
Kindle Papergram needs an independent, decoupled Android-style popup/toast system that:
- Automatically appears and dismisses after a configurable timeout (default 4000ms).
- Can be positioned at the bottom (default) or top of the screen.
- Supports both **Visual Stack** (default: multiple popups stacked with offset) and **Sequential Queue** (one popup at a time, FIFO promotion upon expiry).
- Works globally across any screen/subsystem (e.g. notifications for incoming messages in inactive, unmuted chats).
- Integrates with the event loop in `main.cpp` so popups expire and trigger dirty e-ink refresh even without user input.
- Strictly adheres to Object Calisthenics (0 `else`, max 1 indent level, classes <= 100 lines, methods <= 15 lines, max 2 instance variables, no heap allocations in the inner render loop).

---

## Architecture & Design

### 1. Value Objects & Types (`src/ui/popup_types.h` / `src/ui/popup_types.cpp`)
- `PopupPosition`: `BOTTOM` (default) or `TOP`.
- `PopupMode`: `VISUAL_STACK` (default) or `SEQUENTIAL_QUEUE`.
- `PopupDuration`: wraps duration in milliseconds (default: 4000ms, validates > 0).
- `PopupTimestamp`: monotonic timestamp (`std::chrono::milliseconds`).

### 2. Popup Entry & First-Class Collection (`src/ui/popup_collection.h` / `src/ui/popup_collection.cpp`)
- `PopupEntry`: holds the title, lines, `ToastNotification` card, creation/activation timestamp, duration, and expiration status.
- `PopupCollection`: first-class collection wrapping fixed-capacity `std::array<std::optional<PopupEntry>, 4>` (prevents dynamic heap allocations in render loops).
  - Handles FIFO queue in sequential mode, and stack-drop-oldest overflow in visual stack mode.
  - Exposes active count, visible count, expiration checks, and promotion.

### 3. Layout Calculation (`src/ui/popup_layout.h` / `src/ui/popup_layout.cpp`)
- Computes `BoundingBox` for each popup index based on `PopupPosition` and `PopupMode`.
- Bounds:
  - `BOTTOM`: newest card anchored at `y = 700` (above bottom edge), older cards offset upward by `stack_offset` (48px).
  - `TOP`: newest card anchored at `y = 36` (below `StatusHeader` at y=24), older cards offset downward.
  - Horizontal: centered, e.g. `left = 30`, `right = 570` (540px width).
  - Card height: 70px.
- Computes union `BoundingBox` damage area for partial e-ink refresh.

### 4. ToastNotification Bounds Update (`src/ui/toast_notification.h` / `src/ui/toast_notification.cpp`)
- Add `setBounds(const BoundingBox& bounds)` to `ToastNotification` so cached cards can be repositioned during layout without reallocating strings or vectors.

### 5. PopupManager (`src/ui/popup_manager.h` / `src/ui/popup_manager.cpp`)
- Public API:
  ```cpp
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
      void render(Canvas& canvas) const;

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
      struct State { ... };
      State state_;
  };
  }
  ```
- `notifyIncomingMessage`:
  - Guards against outgoing messages (`message.isOutgoing()`).
  - Guards against active chat (`active_chat_id && message.chatId() == *active_chat_id`).
  - Guards against muted chats (`muted == true`).
  - Displays sender name as title and truncated message text preview as line.

### 6. ScreenNavigator & ConversationScreen Integration
- In `ScreenNavigator`:
  - Store `std::optional<ChatId> active_conversation_id_`.
  - Set it when `openConversation(chat_id, title)` is called.
  - Forward `chat_id` and `title` to `ConversationScreen::setChat(...)`.
  - Clear `active_conversation_id_` on `showLogin()` and `showChatList()`.
  - Expose `activeConversationId() const`.

### 7. Event Loop Integration (`src/main.cpp`)
- Instantiate `PopupManager` at application root.
- Calculate monotonic elapsed time: `now = steady_clock::now() - start_time`.
- In `while (g_running)`:
  - `popup_manager.update(now)`.
  - Input polling with timeout (100ms).
  - If input received: `navigator.handleInput(ev)`, `needs_render = true`.
  - If `popup_manager.needsRedraw()`: `needs_render = true`.
  - When `needs_render`:
    - `navigator.render(canvas)`.
    - `popup_manager.render(canvas)`.
    - Copy to framebuffer, flush.
    - Mark dirty region (full screen on input, or `popup_manager.damageArea()` on popup-only update).
    - Trigger `TypingRefresh` partial update.
    - `popup_manager.acknowledgeRedraw()`.

---

## Step-by-Step Implementation Tasks

### Task 1: Add `PopupTypes` and Extend `ToastNotification`
- Files:
  - Create: `src/ui/popup_types.h`, `src/ui/popup_types.cpp`
  - Modify: `src/ui/toast_notification.h`, `src/ui/toast_notification.cpp`
- Implement:
  - `PopupPosition` enum (`BOTTOM`, `TOP`).
  - `PopupMode` enum (`VISUAL_STACK`, `SEQUENTIAL_QUEUE`).
  - `PopupDuration` and `PopupTimestamp` value objects.
  - `ToastNotification::setBounds(const BoundingBox& bounds)`.

### Task 2: Implement `PopupLayout` and `PopupCollection`
- Files:
  - Create: `src/ui/popup_layout.h`, `src/ui/popup_layout.cpp`
  - Create: `src/ui/popup_collection.h`, `src/ui/popup_collection.cpp`
- Implement:
  - Geometry calculation for stack & top/bottom positions.
  - Fixed-capacity array storage (`std::array<std::optional<PopupEntry>, 4>`).
  - Promotion logic for sequential queue; drop-oldest for visual stack.

### Task 3: Implement `PopupManager`
- Files:
  - Create: `src/ui/popup_manager.h`, `src/ui/popup_manager.cpp`
- Implement:
  - `show(title, lines, duration)`.
  - `notifyIncomingMessage(message, active_chat_id, muted)`.
  - `update(now)` lifecycle, timeout dismissal, and damage calculation.
  - `render(canvas)` without heap allocations.

### Task 4: Unit Test Suite for Popup System
- Files:
  - Create: `tests/test_popup_manager.cpp`
- Tests cover:
  - Default mode (`VISUAL_STACK`) and default position (`BOTTOM`).
  - Auto-dismissal when `now >= created_at + duration`.
  - Position geometry: Top (below header), Bottom (above footer).
  - Visual stack rendering with offsets.
  - Sequential queue: only 1 visible, next promoted when current expires.
  - Incoming message notifications: filter active chat, filter muted, filter outgoing, accept unmuted incoming.
  - Damage area calculation.

### Task 5: ScreenNavigator and ConversationScreen Wire-up
- Files:
  - Modify: `src/ui/screen_navigator.h`, `src/ui/screen_navigator.cpp`
  - Modify: `src/ui/conversation_screen.h`, `src/ui/conversation_screen.cpp`
- Implement:
  - Track `activeConversationId()`.
  - Set chat in `openConversation`.

### Task 6: Main Loop Timer-Driven Integration
- Files:
  - Modify: `src/main.cpp`
- Implement:
  - Monotonic clock updates on every loop iteration.
  - Redraw on `popup_manager.needsRedraw()` even when idle (no input events).
  - Render navigator screen + popup overlay.
  - Dirty tracker partial update with damage area.

---

## Verification Plan

1. **Host Automated Tests**:
   - `make test`: All existing 45 tests + all new `test_popup_manager` tests pass.
   - `make test-asan`: Clean execution under AddressSanitizer and UndefinedBehaviorSanitizer (0 leaks, 0 errors).
2. **Build Verification**:
   - `make client`: Host executable builds without warnings.
   - `make kindle-debug`: Cross-compiles for ARM32 Kindle without errors.
3. **Functional / Device Verification**:
   - Run on host and/or deploy to Kindle `root@192.168.2.2`.
   - Verify popup auto-dismissal after 4 seconds without any keystroke.
   - Verify visual stack and sequential queue transitions.
   - Verify bottom and top placement.
   - Verify incoming message popup triggers for other chats.
