# Technical Design: Kindle Menu Button & Contextual Menu Subsystem

## 1. Executive Summary

This document specifies the technical design for the Kindle Menu Button and Menu subsystem in Papergram. The feature provides a unified, modal Game Boy RPG-inspired dialogue menu accessible from any screen via the Kindle Keyboard's physical **Menu** button (`KEY_MENU`). The menu presents contextual actions relevant to the active screen, followed by consistent global default actions (such as e-ink screen refresh, shortcuts help, logout, and clean shutdown).

---

## 2. Goals & Invariants

### 2.1 Goals
- **Contextual + Global Actions**: Present screen-specific operations alongside global system controls without visual clutter.
- **Game Boy UI/UX Aesthetics**: Adhere strictly to `docs/sot/ui_ux_design.md` with a 4-tone grayscale palette, double-line dialogue card borders, and classic RPG cursor highlights.
- **Physical-First Ergonomics**: Dedicated support for Kindle hardware buttons (D-pad Up/Down with circular wrap-around, Enter/A to execute, Back/B or Menu to dismiss).
- **Zero-Dependency & E-Ink Friendly**: Flat rect fills, sharp contrast, zero ghosting dithering, and low memory footprint (< 50 KB overhead).

### 2.2 Coding Standards & Constraints
- **Object Calisthenics**:
  - Max 1 indentation level per method.
  - No `else` keywords: guard clauses and early returns.
  - Wrap domain primitives (`MenuLabel`, `MenuItemIndex`).
  - First-class collection: `MenuList`.
  - Max 2 instance variables per class.
  - Class length $\le 100$ lines, method length $\le 15$ lines.
  - Fail-fast validation at system boundaries.

---

## 3. UI/UX & Visual Layout

### 3.1 Dimensions and Placement
- **Display Target**: 600×800 8bpp Kindle Keyboard E-Ink Pearl canvas.
- **Menu Card Width**: 460px centered horizontally (`x = 70` to `x = 530`).
- **Menu Card Height**: Dynamically computed:
  $$\text{height} = \text{HEADER\_HEIGHT} (34\text{px}) + (\text{item\_count} \times \text{ROW\_HEIGHT} (36\text{px})) + \text{SEPARATOR\_HEIGHT} (12\text{px if present}) + \text{BOTTOM\_PADDING} (14\text{px})$$
- **Card Placement**: Centered vertically: $y = \frac{800 - \text{height}}{2}$.

### 3.2 Visual Elements & Palette
- **Outer Border**: 1px solid `GrayscaleColor::BLACK`.
- **Inner Border**: 1px solid `GrayscaleColor::DARK_GRAY` (2px margin from outer border).
- **Background**: Solid `GrayscaleColor::LIGHT_GRAY` or `WHITE`.
- **Title Banner**: Centered `=== MENU ===` rendered in `GrayscaleColor::BLACK` with a 1px `GrayscaleColor::DARK_GRAY` bottom rule.
- **Unfocused Items**: Text prefixed with `  ` in `GrayscaleColor::BLACK`.
- **Focused Item (Inverted Contrast)**:
  - Solid rectangular background in `GrayscaleColor::BLACK` across item width.
  - Text rendered in `GrayscaleColor::WHITE`, prefixed with `> ` (or `▶ `).
- **Section Divider**: 1px horizontal line in `GrayscaleColor::DARK_GRAY` between contextual and global items.

---

## 4. Hardware & Input Layer

### 4.1 Evdev Translation (`src/hal/input_device_evdev.cpp`)
- Linux kernel code for the physical Kindle Keyboard Menu button is `139` (`KEY_MENU`).
- In `InputDeviceEvdev::translateSpecial`:
  ```cpp
  if (code == LINUX_KEY_MENU || code == 139) {
      out = KeyCode::KEY_MENU;
      return true;
  }
  ```

### 4.2 Fallback Stdin Translation (`src/hal/fallback_devices.cpp`)
- In `StdinInputDevice::mapAlphaChar`:
  - Mapping `'m'` or `'M'` when outside text-entry modes (or via escape sequence) to `KeyCode::KEY_MENU` allows frictionless testing on developer workstations.

---

## 5. Software Architecture & Class Model

```
+-------------------------------------------------------------+
|                     ScreenNavigator                         |
|  - active_screen_: IScreen*                                 |
|  - menu_: KindleMenu                                        |
+-------------------------------------------------------------+
               |                               |
               | queries                       | renders & delegates
               v                               v
+-----------------------------+   +---------------------------+
|          IScreen            |   |        KindleMenu         |
| + contextualMenuItems()     |   | - items_: MenuList        |
+-----------------------------+   | - visible_: bool          |
               ^                  +---------------------------+
               |                               |
   +-----------+-----------+                   | contains
   |           |           |                   v
[Login]    [ChatList]  [Conversation]   +---------------------+
                                        |      MenuList       |
                                        | - items: vector     |
                                        | - selected: size_t  |
                                        +---------------------+
                                                   |
                                                   v
                                        +---------------------+
                                        |      MenuItem       |
                                        | - label: MenuLabel  |
                                        | - action: ActionFn  |
                                        | - is_separator: bool|
                                        +---------------------+
```

### 5.1 `MenuLabel` Value Object (`src/ui/menu_label.h/.cpp`)
- Enforces non-empty, trimmed string up to 40 characters.
- Throws `std::invalid_argument` if string is empty or whitespace-only.

### 5.2 `MenuItem` (`src/ui/menu_item.h/.cpp`)
- Stores:
  - `MenuLabel label_`
  - `std::function<void()> action_`
  - `bool is_separator_before_`
- Methods:
  - `execute() const`: calls `action_()`.
  - `label() const -> const std::string&`
  - `isSeparatorBefore() const -> bool`

### 5.3 `MenuList` (`src/ui/menu_list.h/.cpp`)
- First-class collection wrapping items.
- Instance variables:
  - `std::vector<MenuItem> items_`
  - `std::size_t selected_index_`
- Methods:
  - `append(const MenuItem& item)`
  - `selectNext()`: increments index with wrap-around to 0.
  - `selectPrevious()`: decrements index with wrap-around to `count() - 1`.
  - `selectedIndex() const -> std::size_t`
  - `selectedItem() const -> const MenuItem&`
  - `count() const -> std::size_t`
  - `clear()`

### 5.4 `KindleMenu` (`src/ui/kindle_menu.h/.cpp`)
- Instance variables:
  - `MenuList items_`
  - `bool is_open_`
- Methods:
  - `open(const MenuList& items)`: sets items, resets selection to 0, sets `is_open_ = true`.
  - `close()`: sets `is_open_ = false`.
  - `isOpen() const -> bool`
  - `handleInput(const InputEvent& event) -> bool`: returns `true` if event was consumed.
    - `KEY_UP`: calls `items_.selectPrevious()`.
    - `KEY_DOWN`: calls `items_.selectNext()`.
    - `KEY_ENTER`: calls `items_.selectedItem().execute()` then `close()`.
    - `KEY_BACK`: calls `close()`.
    - `KEY_MENU`: calls `close()` (toggle behavior).
  - `render(Canvas& canvas) const`: renders the Game Boy dialogue box, item list, separator, and active selection bar.

### 5.5 `IScreen` Contextual Interface (`src/ui/screen.h`)
- Extends `IScreen` with:
  ```cpp
  virtual std::vector<MenuItem> contextualMenuItems() const { return {}; }
  ```

### 5.6 `ScreenNavigator` Orchestration (`src/ui/screen_navigator.h/.cpp`)
- In `handleInput(const InputEvent& event)`:
  - If `event.pressed && event.code == KeyCode::KEY_MENU`:
    - If `menu_.isOpen()`, call `menu_.close()`.
    - Else, call `openMenuForActiveScreen()`.
    - Return early (input consumed).
  - If `menu_.isOpen()`:
    - Pass event to `menu_.handleInput(event)`.
    - Return early so underlying screen does not receive input while menu is open.
  - If menu is closed:
    - Forward event to `active_screen_->handleInput(event)`.
- In `render(Canvas& canvas)`:
  - Call `active_screen_->render(canvas)`.
  - If `menu_.isOpen()`, call `menu_.render(canvas)`.

---

## 6. Detailed Menu Actions by Screen

### 6.1 `LoginScreen` Contextual Actions
1. **"Reset Form"**: Clears current input buffer and resets login state to step 0 (`+`).
2. **"Keyboard Tips"**: Displays the ALT-key number entry toast notification.

### 6.2 `ChatListScreen` Contextual Actions
1. **"Refresh Chats"**: Calls `client_->getDialogs(chats_)` to refresh MTProto dialogs.
2. **"Mark All Read"**: Resets unread counters in the local dialog list.

### 6.3 `ConversationScreen` Contextual Actions
1. **"Refresh History"**: Calls `client_->getHistory(active_chat_id, history_)`.
2. **"Clear Input"**: Wipes the message input bar buffer.
3. **"Back to Chats"**: Calls `navigator_->showChatList()`.

### 6.4 Global Default Actions (Appended to Every Screen)
1. **"Shortcuts & Help"**: Displays quick help tips.
2. **"Refresh Screen"**: Forces a full e-ink GC16 clear waveform to purge ghosting.
3. **"Log Out"**: Clears stored session credentials and navigates to `showLogin()`.
4. **"Exit Papergram"**: Exits application loop gracefully.

---

## 7. Testing Strategy

### 7.1 Unit Tests (`tests/test_kindle_menu.cpp`)
1. **`MenuLabel` Validation**: Verifies rejection of empty strings and whitespace-only labels.
2. **`MenuList` Navigation**:
   - Wrap-around when moving next past last item.
   - Wrap-around when moving previous past first item.
   - Out-of-bounds safety.
3. **`KindleMenu` State & Interaction**:
   - `open()` sets `isOpen() == true` and selected index to 0.
   - `KEY_UP` and `KEY_DOWN` update selected item.
   - `KEY_ENTER` invokes action callback and closes menu.
   - `KEY_BACK` closes menu without invoking action.
   - `KEY_MENU` toggles menu closed.
4. **`ScreenNavigator` Menu Routing**:
   - Verifies `KEY_MENU` opens menu with active screen's items + global defaults.
   - Verifies modal input swallows keystrokes while menu is open.
   - Verifies menu overlay renders over active screen.
5. **HAL evdev Translation (`tests/test_input_device_evdev.cpp`)**:
   - Verifies Linux code 139 translates to `KeyCode::KEY_MENU`.
