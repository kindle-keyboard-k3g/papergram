# Kindle Menu Button & Contextual Menu Subsystem Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the physical Kindle Menu button support (`KEY_MENU`) and a Game Boy RPG-inspired modal contextual dialogue menu across `LoginScreen`, `ChatListScreen`, and `ConversationScreen`, managed centrally by `ScreenNavigator`.

**Architecture:** 
- A value object `MenuLabel` validates non-empty menu text.
- `MenuItem` pairs a label with an executable action (`std::function<void()>`) and an optional section divider flag.
- `MenuList` is a first-class collection providing circular D-pad wrap-around selection.
- `KindleMenu` renders a centered double-line border dialog card with inverted contrast selection highlight and captures modal input.
- `IScreen` exposes `contextualMenuItems()`, which `ScreenNavigator` queries upon `KEY_MENU` press, appending global default actions ("Shortcuts & Help", "Refresh Screen", "Log Out", "Exit Papergram").
- `InputDeviceEvdev` maps Linux keycode `139` to `KeyCode::KEY_MENU`, and `StdinInputDevice` maps `'m'` / `'M'` to `KeyCode::KEY_MENU` for development host testing.

**Tech Stack:** C++17, GCC/Clang, POSIX evdev & poll, custom Canvas 8bpp graphics, Linux fb0.

**Spec:** `docs/superpowers/specs/2026-09-21-kindle-menu-button-design.md`

## Global Constraints

- Max 1 indentation level per method (extract helper methods).
- No `else` keywords: use guard clauses, early returns, or polymorphism.
- Wrap domain primitives (`MenuLabel`).
- First-class collection: `MenuList`.
- Keep entities small: classes $\le 100$ lines, methods $\le 15$ lines, $\le 2$ instance variables per class.
- Tell, don't ask: expose behavior rather than raw internals.
- 4-Tone Grayscale: strictly constrain UI rendering to `GrayscaleColor::WHITE`, `LIGHT_GRAY`, `DARK_GRAY`, and `BLACK`.
- Double-border dialogue cards: outer black border, 2px inner dark gray border, light gray or white fill.
- All tests must pass cleanly with `make test`.

---

### Task 1: Hardware Input Mapping for KEY_MENU

**Files:**
- Modify: `src/hal/input_device_evdev.cpp:111-124`
- Modify: `src/hal/fallback_devices.cpp:140-150`
- Test: `tests/test_input_evdev.cpp`
- Test: `tests/test_fallback_devices.cpp`

**Interfaces:**
- Consumes: `LINUX_KEY_MENU` (139), `KeyCode::KEY_MENU` in `src/hal/input_device.h`
- Produces: `InputDeviceEvdev` and `StdinInputDevice` emitting `KeyCode::KEY_MENU`

- [ ] **Step 1: Write the failing tests in `tests/test_input_evdev.cpp` and `tests/test_fallback_devices.cpp`**

In `tests/test_input_evdev.cpp`:
```cpp
TEST(input_device_evdev_translates_menu_key) {
    InputDeviceEvdev device;
    InputEvent ev;
    // Test code 139 (KEY_MENU)
    ASSERT_TRUE(device.translateCode(1, 139, 1, ev));
    ASSERT_TRUE(ev.pressed);
    ASSERT_EQ(static_cast<int>(KeyCode::KEY_MENU), static_cast<int>(ev.code));
}
```

In `tests/test_fallback_devices.cpp`:
```cpp
TEST(stdin_input_device_maps_m_to_key_menu) {
    StdinInputDevice device;
    InputEvent ev;
    ASSERT_TRUE(device.mapCharToEvent('m', ev));
    ASSERT_TRUE(ev.pressed);
    ASSERT_EQ(static_cast<int>(KeyCode::KEY_MENU), static_cast<int>(ev.code));

    ASSERT_TRUE(device.mapCharToEvent('M', ev));
    ASSERT_TRUE(ev.pressed);
    ASSERT_EQ(static_cast<int>(KeyCode::KEY_MENU), static_cast<int>(ev.code));
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `make test`
Expected: FAIL with compilation error or assertion failure because `139` and `'m'` are not mapped to `KEY_MENU`.

- [ ] **Step 3: Implement minimal input mapping**

In `src/hal/input_device_evdev.cpp`:
Add Linux Menu key constant and handling in `translateSpecial`:
```cpp
constexpr std::uint16_t LINUX_KEY_MENU = 139;

bool translateSpecial(std::uint16_t code, KeyCode& out) {
    if (translatePower(code, out)) return true;
    if (code == LINUX_KEY_MENU || code == 139) {
        out = KeyCode::KEY_MENU;
        return true;
    }
    // ... rest of translateSpecial
```

In `src/hal/fallback_devices.cpp`:
In `StdinInputDevice::mapCharToEvent` or `mapAlphaChar`:
Ensure `'m'` and `'M'` map to `KeyCode::KEY_MENU` when handling menu key, or in `mapCharToEvent`:
```cpp
bool StdinInputDevice::mapCharToEvent(char ch, InputEvent& out) {
    out.pressed = true;
    if (ch == 27) return parseEscapeSequence(out);
    if (ch == 4) { closed_ = true; return false; }
    if (ch == '`' || ch == '~') { out.code = KeyCode::KEY_POWER; return true; }
    if (ch == 'm' || ch == 'M') { out.code = KeyCode::KEY_MENU; return true; }
    if (mapAlphaChar(ch, out)) return true;
    return mapDigitOrControl(ch, out);
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `make test`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/hal/input_device_evdev.cpp src/hal/fallback_devices.cpp tests/test_input_evdev.cpp tests/test_fallback_devices.cpp
git commit -m "feat(hal): map KEY_MENU for evdev and fallback stdin device"
```

---

### Task 2: MenuLabel Value Object

**Files:**
- Create: `src/ui/menu_label.h`
- Create: `src/ui/menu_label.cpp`
- Test: `tests/test_menu_label.cpp`

**Interfaces:**
- Consumes: none
- Produces: `ui::MenuLabel` class with `const std::string& value() const`

- [ ] **Step 1: Write the failing tests in `tests/test_menu_label.cpp`**

```cpp
#include "test_framework.h"
#include "../src/ui/menu_label.h"

#include <stdexcept>

TEST(menu_label_accepts_valid_string) {
    ui::MenuLabel label("Refresh Screen");
    ASSERT_EQ(std::string("Refresh Screen"), label.value());
}

TEST(menu_label_rejects_empty_or_whitespace_string) {
    ASSERT_THROW(ui::MenuLabel(""), std::invalid_argument);
    ASSERT_THROW(ui::MenuLabel("   "), std::invalid_argument);
}

TEST(menu_label_trims_surrounding_whitespace) {
    ui::MenuLabel label("  Log Out  ");
    ASSERT_EQ(std::string("Log Out"), label.value());
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `make test`
Expected: FAIL with compilation error (file not found).

- [ ] **Step 3: Implement minimal `MenuLabel`**

`src/ui/menu_label.h`:
```cpp
#ifndef KINDLE_UI_MENU_LABEL_H
#define KINDLE_UI_MENU_LABEL_H

#include <string>

namespace ui {

class MenuLabel {
public:
    explicit MenuLabel(const std::string& text);

    const std::string& value() const;

private:
    std::string value_;
};

} // namespace ui

#endif
```

`src/ui/menu_label.cpp`:
```cpp
#include "menu_label.h"
#include <stdexcept>

namespace {
std::string trim(const std::string& str) {
    std::size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    std::size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
} // namespace

namespace ui {

MenuLabel::MenuLabel(const std::string& text) : value_(trim(text)) {
    if (value_.empty()) {
        throw std::invalid_argument("Menu label cannot be empty or whitespace");
    }
}

const std::string& MenuLabel::value() const {
    return value_;
}

} // namespace ui
```

- [ ] **Step 4: Run test to verify it passes**

Run: `make test`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/ui/menu_label.h src/ui/menu_label.cpp tests/test_menu_label.cpp
git commit -m "feat(ui): add MenuLabel value object with trimming and validation"
```

---

### Task 3: MenuItem & MenuList

**Files:**
- Create: `src/ui/menu_item.h`
- Create: `src/ui/menu_item.cpp`
- Create: `src/ui/menu_list.h`
- Create: `src/ui/menu_list.cpp`
- Test: `tests/test_menu_list.cpp`

**Interfaces:**
- Consumes: `ui::MenuLabel`
- Produces: `ui::MenuItem` and `ui::MenuList`

- [ ] **Step 1: Write the failing tests in `tests/test_menu_list.cpp`**

```cpp
#include "test_framework.h"
#include "../src/ui/menu_item.h"
#include "../src/ui/menu_list.h"

TEST(menu_item_stores_label_and_executes_action) {
    bool executed = false;
    ui::MenuItem item(ui::MenuLabel("Refresh"), [&executed]() { executed = true; }, false);

    ASSERT_EQ(std::string("Refresh"), item.label().value());
    ASSERT_FALSE(item.isSeparatorBefore());

    item.execute();
    ASSERT_TRUE(executed);
}

TEST(menu_list_handles_empty_state) {
    ui::MenuList list;
    ASSERT_EQ(0U, list.count());
    ASSERT_FALSE(list.hasSelection());
}

TEST(menu_list_navigates_with_wraparound) {
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Item 1"), []() {}));
    list.append(ui::MenuItem(ui::MenuLabel("Item 2"), []() {}));
    list.append(ui::MenuItem(ui::MenuLabel("Item 3"), []() {}));

    ASSERT_EQ(3U, list.count());
    ASSERT_EQ(0U, list.selectedIndex());
    ASSERT_EQ(std::string("Item 1"), list.selectedItem().label().value());

    list.selectNext();
    ASSERT_EQ(1U, list.selectedIndex());

    list.selectNext();
    ASSERT_EQ(2U, list.selectedIndex());

    // Wrap around to 0
    list.selectNext();
    ASSERT_EQ(0U, list.selectedIndex());

    // Wrap around backward to 2
    list.selectPrevious();
    ASSERT_EQ(2U, list.selectedIndex());
}

TEST(menu_list_clear_resets_state) {
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Item 1"), []() {}));
    list.clear();
    ASSERT_EQ(0U, list.count());
    ASSERT_FALSE(list.hasSelection());
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `make test`
Expected: FAIL with compilation error (files not found).

- [ ] **Step 3: Implement `MenuItem` and `MenuList`**

`src/ui/menu_item.h`:
```cpp
#ifndef KINDLE_UI_MENU_ITEM_H
#define KINDLE_UI_MENU_ITEM_H

#include "menu_label.h"
#include <functional>

namespace ui {

class MenuItem {
public:
    using Action = std::function<void()>;

    MenuItem(MenuLabel label, Action action, bool separator_before = false);

    void execute() const;
    const MenuLabel& label() const;
    bool isSeparatorBefore() const;

private:
    struct ItemData {
        MenuLabel label;
        Action action;
        bool separator_before;
    };

    ItemData data_;
};

} // namespace ui

#endif
```

`src/ui/menu_item.cpp`:
```cpp
#include "menu_item.h"

namespace ui {

MenuItem::MenuItem(MenuLabel label, Action action, bool separator_before)
    : data_{std::move(label), std::move(action), separator_before} {}

void MenuItem::execute() const {
    if (data_.action) {
        data_.action();
    }
}

const MenuLabel& MenuItem::label() const {
    return data_.label;
}

bool MenuItem::isSeparatorBefore() const {
    return data_.separator_before;
}

} // namespace ui
```

`src/ui/menu_list.h`:
```cpp
#ifndef KINDLE_UI_MENU_LIST_H
#define KINDLE_UI_MENU_LIST_H

#include "menu_item.h"
#include <vector>
#include <cstddef>

namespace ui {

class MenuList {
public:
    MenuList();

    void append(const MenuItem& item);
    void clear();

    void selectNext();
    void selectPrevious();
    void resetSelection();

    std::size_t selectedIndex() const;
    const MenuItem& selectedItem() const;
    const MenuItem& at(std::size_t index) const;
    std::size_t count() const;
    bool hasSelection() const;

private:
    std::vector<MenuItem> items_;
    std::size_t selected_index_ = 0U;
};

} // namespace ui

#endif
```

`src/ui/menu_list.cpp`:
```cpp
#include "menu_list.h"
#include <stdexcept>

namespace ui {

MenuList::MenuList() = default;

void MenuList::append(const MenuItem& item) {
    items_.push_back(item);
}

void MenuList::clear() {
    items_.clear();
    selected_index_ = 0U;
}

void MenuList::selectNext() {
    if (items_.empty()) return;
    selected_index_ = (selected_index_ + 1U) % items_.size();
}

void MenuList::selectPrevious() {
    if (items_.empty()) return;
    selected_index_ = (selected_index_ + items_.size() - 1U) % items_.size();
}

void MenuList::resetSelection() {
    selected_index_ = 0U;
}

std::size_t MenuList::selectedIndex() const {
    return selected_index_;
}

const MenuItem& MenuList::selectedItem() const {
    return at(selected_index_);
}

const MenuItem& MenuList::at(std::size_t index) const {
    if (index >= items_.size()) {
        throw std::out_of_range("Menu index out of range");
    }
    return items_[index];
}

std::size_t MenuList::count() const {
    return items_.size();
}

bool MenuList::hasSelection() const {
    return !items_.empty();
}

} // namespace ui
```

- [ ] **Step 4: Run test to verify it passes**

Run: `make test`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/ui/menu_item.h src/ui/menu_item.cpp src/ui/menu_list.h src/ui/menu_list.cpp tests/test_menu_list.cpp
git commit -m "feat(ui): add MenuItem and MenuList first-class collection"
```

---

### Task 4: KindleMenu Modal Component & Rendering

**Files:**
- Create: `src/ui/kindle_menu.h`
- Create: `src/ui/kindle_menu.cpp`
- Test: `tests/test_kindle_menu.cpp`

**Interfaces:**
- Consumes: `ui::MenuList`, `Canvas`, `InputEvent`
- Produces: `ui::KindleMenu`

- [ ] **Step 1: Write the failing tests in `tests/test_kindle_menu.cpp`**

```cpp
#include "test_framework.h"
#include "../src/ui/kindle_menu.h"
#include "../src/graphics/canvas.h"

TEST(kindle_menu_initial_state_is_closed) {
    ui::KindleMenu menu;
    ASSERT_FALSE(menu.isOpen());
}

TEST(kindle_menu_open_and_close) {
    ui::KindleMenu menu;
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Option 1"), []() {}));
    
    menu.open(list);
    ASSERT_TRUE(menu.isOpen());
    
    menu.close();
    ASSERT_FALSE(menu.isOpen());
}

TEST(kindle_menu_handles_navigation_and_execution) {
    ui::KindleMenu menu;
    bool action1_ran = false;
    bool action2_ran = false;
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Action 1"), [&]() { action1_ran = true; }));
    list.append(ui::MenuItem(ui::MenuLabel("Action 2"), [&]() { action2_ran = true; }));

    menu.open(list);

    // Down arrow moves to Action 2
    InputEvent down_ev{KeyCode::KEY_DOWN, true};
    ASSERT_TRUE(menu.handleInput(down_ev));

    // Enter executes Action 2 and closes menu
    InputEvent enter_ev{KeyCode::KEY_ENTER, true};
    ASSERT_TRUE(menu.handleInput(enter_ev));
    ASSERT_TRUE(action2_ran);
    ASSERT_FALSE(action1_ran);
    ASSERT_FALSE(menu.isOpen());
}

TEST(kindle_menu_dismiss_with_back_and_menu_key) {
    ui::KindleMenu menu;
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Option"), []() {}));

    menu.open(list);
    InputEvent back_ev{KeyCode::KEY_BACK, true};
    ASSERT_TRUE(menu.handleInput(back_ev));
    ASSERT_FALSE(menu.isOpen());

    menu.open(list);
    InputEvent menu_ev{KeyCode::KEY_MENU, true};
    ASSERT_TRUE(menu.handleInput(menu_ev));
    ASSERT_FALSE(menu.isOpen());
}

TEST(kindle_menu_renders_double_border_and_items_on_canvas) {
    Canvas canvas;
    ui::KindleMenu menu;
    ui::MenuList list;
    list.append(ui::MenuItem(ui::MenuLabel("Item 1"), []() {}));
    list.append(ui::MenuItem(ui::MenuLabel("Item 2"), []() {}, true)); // with separator

    menu.open(list);
    menu.render(canvas);

    // Verify canvas rendered without crash and menu remains open
    ASSERT_TRUE(menu.isOpen());
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `make test`
Expected: FAIL with compilation error (files not found).

- [ ] **Step 3: Implement `KindleMenu`**

`src/ui/kindle_menu.h`:
```cpp
#ifndef KINDLE_UI_KINDLE_MENU_H
#define KINDLE_UI_KINDLE_MENU_H

#include "menu_list.h"
#include "../graphics/canvas.h"
#include "../hal/input_device.h"

namespace ui {

class KindleMenu {
public:
    KindleMenu();

    void open(const MenuList& items);
    void close();
    bool isOpen() const;

    bool handleInput(const InputEvent& event);
    void render(Canvas& canvas) const;

private:
    struct MenuState {
        MenuList items;
        bool open = false;
    };

    MenuState state_;

    void renderCard(Canvas& canvas, const BoundingBox& box) const;
    void renderTitle(Canvas& canvas, const BoundingBox& box) const;
    void renderItems(Canvas& canvas, int start_y, int left_x, int right_x) const;
    void renderItemRow(Canvas& canvas, std::size_t index, int y, int left_x, int right_x) const;
    static BoundingBox computeBounds(std::size_t item_count, bool has_separator);
};

} // namespace ui

#endif
```

`src/ui/kindle_menu.cpp`:
```cpp
#include "kindle_menu.h"
#include "../graphics/bitmap_font.h"

namespace {
constexpr int MENU_WIDTH = 460;
constexpr int MENU_LEFT = 70;
constexpr int HEADER_HEIGHT = 38;
constexpr int ITEM_HEIGHT = 36;
constexpr int SEPARATOR_HEIGHT = 10;
constexpr int PADDING_BOTTOM = 14;
} // namespace

namespace ui {

KindleMenu::KindleMenu() = default;

void KindleMenu::open(const MenuList& items) {
    state_.items = items;
    state_.items.resetSelection();
    state_.open = true;
}

void KindleMenu::close() {
    state_.open = false;
}

bool KindleMenu::isOpen() const {
    return state_.open;
}

bool KindleMenu::handleInput(const InputEvent& event) {
    if (!state_.open || !event.pressed) return false;
    if (event.code == KeyCode::KEY_BACK || event.code == KeyCode::KEY_MENU) {
        close();
        return true;
    }
    if (event.code == KeyCode::KEY_UP) {
        state_.items.selectPrevious();
        return true;
    }
    if (event.code == KeyCode::KEY_DOWN) {
        state_.items.selectNext();
        return true;
    }
    if (event.code == KeyCode::KEY_ENTER && state_.items.hasSelection()) {
        const MenuItem& selected = state_.items.selectedItem();
        close();
        selected.execute();
        return true;
    }
    return true; // Consume any other input while modal menu is open
}

BoundingBox KindleMenu::computeBounds(std::size_t item_count, bool has_separator) {
    int height = HEADER_HEIGHT + (static_cast<int>(item_count) * ITEM_HEIGHT) + PADDING_BOTTOM;
    if (has_separator) height += SEPARATOR_HEIGHT;
    int top = (800 - height) / 2;
    return BoundingBox(MENU_LEFT, top, MENU_LEFT + MENU_WIDTH, top + height);
}

void KindleMenu::render(Canvas& canvas) const {
    if (!state_.open || state_.items.count() == 0) return;
    bool has_sep = false;
    for (std::size_t i = 0; i < state_.items.count(); ++i) {
        if (state_.items.at(i).isSeparatorBefore()) has_sep = true;
    }
    BoundingBox box = computeBounds(state_.items.count(), has_sep);
    renderCard(canvas, box);
    renderTitle(canvas, box);
    renderItems(canvas, box.top() + HEADER_HEIGHT, box.left() + 10, box.right() - 10);
}

void KindleMenu::renderCard(Canvas& canvas, const BoundingBox& box) const {
    canvas.fillRect(box, GrayscaleColor::WHITE);
    canvas.drawRect(box, GrayscaleColor::BLACK);
    canvas.drawRect(BoundingBox(box.left() + 2, box.top() + 2, box.right() - 2, box.bottom() - 2),
                    GrayscaleColor::DARK_GRAY);
}

void KindleMenu::renderTitle(Canvas& canvas, const BoundingBox& box) const {
    canvas.fillRect(BoundingBox(box.left() + 3, box.top() + 3, box.right() - 3, box.top() + HEADER_HEIGHT - 4),
                    GrayscaleColor::LIGHT_GRAY);
    canvas.blitText(ScreenCoordinate(box.left() + 160, box.top() + 10),
                    "=== MENU ===", GrayscaleColor::BLACK);
    canvas.drawLine(ScreenCoordinate(box.left() + 3, box.top() + HEADER_HEIGHT - 3),
                    ScreenCoordinate(box.right() - 3, box.top() + HEADER_HEIGHT - 3),
                    GrayscaleColor::BLACK);
}

void KindleMenu::renderItems(Canvas& canvas, int start_y, int left_x, int right_x) const {
    int current_y = start_y;
    for (std::size_t i = 0; i < state_.items.count(); ++i) {
        if (state_.items.at(i).isSeparatorBefore()) {
            canvas.drawLine(ScreenCoordinate(left_x, current_y + 4),
                            ScreenCoordinate(right_x, current_y + 4),
                            GrayscaleColor::DARK_GRAY);
            current_y += SEPARATOR_HEIGHT;
        }
        renderItemRow(canvas, i, current_y, left_x, right_x);
        current_y += ITEM_HEIGHT;
    }
}

void KindleMenu::renderItemRow(Canvas& canvas, std::size_t index, int y, int left_x, int right_x) const {
    bool is_selected = (index == state_.items.selectedIndex());
    if (is_selected) {
        canvas.fillRect(BoundingBox(left_x, y + 2, right_x, y + ITEM_HEIGHT - 2),
                        GrayscaleColor::BLACK);
        std::string text = "> " + state_.items.at(index).label().value();
        canvas.blitText(ScreenCoordinate(left_x + 12, y + 10), text, GrayscaleColor::WHITE);
        return;
    }
    std::string text = "  " + state_.items.at(index).label().value();
    canvas.blitText(ScreenCoordinate(left_x + 12, y + 10), text, GrayscaleColor::BLACK);
}

} // namespace ui
```

- [ ] **Step 4: Run test to verify it passes**

Run: `make test`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/ui/kindle_menu.h src/ui/kindle_menu.cpp tests/test_kindle_menu.cpp
git commit -m "feat(ui): add KindleMenu modal component with Game Boy dialogue box styling"
```

---

### Task 5: Contextual Menu Provider in IScreen & Screens

**Files:**
- Modify: `src/ui/screen.h:7-17`
- Modify: `src/ui/login_screen.h`
- Modify: `src/ui/login_screen.cpp`
- Modify: `src/ui/chat_list_screen.h`
- Modify: `src/ui/chat_list_screen.cpp`
- Modify: `src/ui/conversation_screen.h`
- Modify: `src/ui/conversation_screen.cpp`
- Test: `tests/test_screen_menu_items.cpp`

**Interfaces:**
- Consumes: `ui::MenuItem`
- Produces: `IScreen::contextualMenuItems() const -> std::vector<ui::MenuItem>` implemented across screens

- [ ] **Step 1: Write the failing tests in `tests/test_screen_menu_items.cpp`**

```cpp
#include "test_framework.h"
#include "../src/ui/login_screen.h"
#include "../src/ui/chat_list_screen.h"
#include "../src/ui/conversation_screen.h"
#include "../src/ui/screen_navigator.h"
#include "../src/mtproto/telegram_client.h"
#include "../src/hal/fallback_devices.h"

TEST(login_screen_provides_contextual_menu_items) {
    DummyTransport transport;
    mtproto::TelegramClient client(transport);
    ScreenNavigator navigator;
    LoginScreen login(client, navigator);

    auto items = login.contextualMenuItems();
    ASSERT_TRUE(items.size() >= 2U);
    ASSERT_EQ(std::string("Reset Form"), items[0].label().value());
    ASSERT_EQ(std::string("Keyboard Tips"), items[1].label().value());
}

TEST(chat_list_screen_provides_contextual_menu_items) {
    DummyTransport transport;
    mtproto::TelegramClient client(transport);
    ScreenNavigator navigator;
    ChatListScreen chat_list(client, navigator);

    auto items = chat_list.contextualMenuItems();
    ASSERT_TRUE(items.size() >= 2U);
    ASSERT_EQ(std::string("Refresh Chats"), items[0].label().value());
    ASSERT_EQ(std::string("Mark All Read"), items[1].label().value());
}

TEST(conversation_screen_provides_contextual_menu_items) {
    DummyTransport transport;
    mtproto::TelegramClient client(transport);
    ScreenNavigator navigator;
    ConversationScreen conv(client, navigator);

    auto items = conv.contextualMenuItems();
    ASSERT_TRUE(items.size() >= 3U);
    ASSERT_EQ(std::string("Refresh History"), items[0].label().value());
    ASSERT_EQ(std::string("Clear Input"), items[1].label().value());
    ASSERT_EQ(std::string("Back to Chats"), items[2].label().value());
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `make test`
Expected: FAIL with compilation error (`contextualMenuItems` not found).

- [ ] **Step 3: Update `IScreen` and implement in each screen**

In `src/ui/screen.h`:
```cpp
#include "menu_item.h"
#include <vector>

class IScreen {
public:
    virtual ~IScreen() = default;

    virtual void render(Canvas& canvas) = 0;
    virtual void handleInput(const InputEvent& event) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual std::vector<ui::MenuItem> contextualMenuItems() { return {}; }
};
```

In `src/ui/login_screen.h` and `.cpp`:
Implement:
```cpp
std::vector<ui::MenuItem> LoginScreen::contextualMenuItems() override {
    std::vector<ui::MenuItem> items;
    items.emplace_back(ui::MenuLabel("Reset Form"), [this]() {
        state_.step = 0;
        state_.buffer = "+";
        state_.status_message = "Enter your phone number (+1234...)";
    });
    items.emplace_back(ui::MenuLabel("Keyboard Tips"), [this]() {
        // Toggle or ensure tips toast is triggered
        state_.step = 0;
    });
    return items;
}
```

In `src/ui/chat_list_screen.h` and `.cpp`:
Implement:
```cpp
std::vector<ui::MenuItem> ChatListScreen::contextualMenuItems() override {
    std::vector<ui::MenuItem> items;
    items.emplace_back(ui::MenuLabel("Refresh Chats"), [this]() {
        if (state_.client) state_.client->getDialogs(state_.chats);
    });
    items.emplace_back(ui::MenuLabel("Mark All Read"), [this]() {
        // Reset local unread badges
        for (std::size_t i = 0; i < state_.chats.count(); ++i) {
            state_.chats.at(i).markAsRead();
        }
    });
    return items;
}
```

In `src/ui/conversation_screen.h` and `.cpp`:
Implement:
```cpp
std::vector<ui::MenuItem> ConversationScreen::contextualMenuItems() override {
    std::vector<ui::MenuItem> items;
    items.emplace_back(ui::MenuLabel("Refresh History"), [this]() {
        if (state_.client) state_.client->getHistory(state_.active_chat_id, state_.history);
    });
    items.emplace_back(ui::MenuLabel("Clear Input"), [this]() {
        state_.input_buffer.clear();
    });
    items.emplace_back(ui::MenuLabel("Back to Chats"), [this]() {
        if (state_.navigator) state_.navigator->showChatList();
    });
    return items;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `make test`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/ui/screen.h src/ui/login_screen.h src/ui/login_screen.cpp src/ui/chat_list_screen.h src/ui/chat_list_screen.cpp src/ui/conversation_screen.h src/ui/conversation_screen.cpp tests/test_screen_menu_items.cpp
git commit -m "feat(ui): add contextualMenuItems to IScreen, LoginScreen, ChatListScreen, and ConversationScreen"
```

---

### Task 6: ScreenNavigator Integration & Global Actions

**Files:**
- Modify: `src/ui/screen_navigator.h`
- Modify: `src/ui/screen_navigator.cpp`
- Test: `tests/test_screen_navigator_menu.cpp`

**Interfaces:**
- Consumes: `ui::KindleMenu`, `ui::MenuList`, `ui::MenuItem`
- Produces: `ScreenNavigator` modal menu trigger on `KEY_MENU`, global actions ("Shortcuts & Help", "Refresh Screen", "Log Out", "Exit Papergram"), and overlay rendering

- [ ] **Step 1: Write the failing tests in `tests/test_screen_navigator_menu.cpp`**

```cpp
#include "test_framework.h"
#include "../src/ui/screen_navigator.h"
#include "../src/graphics/canvas.h"

namespace {
class DummyMenuScreen : public IScreen {
public:
    int render_count = 0;
    int input_count = 0;

    void render(Canvas&) override { ++render_count; }
    void handleInput(const InputEvent&) override { ++input_count; }
    void onEnter() override {}
    void onExit() override {}
    std::vector<ui::MenuItem> contextualMenuItems() override {
        std::vector<ui::MenuItem> items;
        items.emplace_back(ui::MenuLabel("Context Item"), []() {});
        return items;
    }
};
}

TEST(screen_navigator_key_menu_toggles_kindle_menu) {
    ScreenNavigator navigator;
    auto screen = std::make_unique<DummyMenuScreen>();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    ASSERT_FALSE(navigator.isMenuOpen());

    InputEvent menu_ev{KeyCode::KEY_MENU, true};
    navigator.handleInput(menu_ev);
    ASSERT_TRUE(navigator.isMenuOpen());

    navigator.handleInput(menu_ev);
    ASSERT_FALSE(navigator.isMenuOpen());
}

TEST(screen_navigator_swallows_underlying_input_while_menu_is_open) {
    ScreenNavigator navigator;
    auto screen = std::make_unique<DummyMenuScreen>();
    auto* screen_ptr = screen.get();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    navigator.handleInput(InputEvent{KeyCode::KEY_MENU, true});
    ASSERT_TRUE(navigator.isMenuOpen());

    // Send a typing key or D-pad Up
    navigator.handleInput(InputEvent{KeyCode::KEY_A, true});
    // Screen should NOT have received input
    ASSERT_EQ(0, screen_ptr->input_count);
}

TEST(screen_navigator_renders_menu_overlay_when_open) {
    Canvas canvas;
    ScreenNavigator navigator;
    auto screen = std::make_unique<DummyMenuScreen>();
    auto* screen_ptr = screen.get();
    navigator.setScreens(std::move(screen), nullptr, nullptr);

    navigator.handleInput(InputEvent{KeyCode::KEY_MENU, true});
    navigator.render(canvas);

    ASSERT_EQ(1, screen_ptr->render_count);
    ASSERT_TRUE(navigator.isMenuOpen());
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `make test`
Expected: FAIL with compilation error (`isMenuOpen` not found on `ScreenNavigator`).

- [ ] **Step 3: Implement menu handling in `ScreenNavigator`**

In `src/ui/screen_navigator.h`:
```cpp
#include "kindle_menu.h"
// ...
class ScreenNavigator {
public:
    // ...
    bool isMenuOpen() const;
    void openMenu();
    void closeMenu();
    void setRefreshCallback(std::function<void()> on_refresh);
    void setExitCallback(std::function<void()> on_exit);

private:
    struct NavigationState {
        std::unique_ptr<IScreen> login;
        std::unique_ptr<IScreen> chat_list;
        std::unique_ptr<IScreen> conversation;
        std::unique_ptr<IScreen> screensaver;
        IScreen* active_screen = nullptr;
        IScreen* previous_active_screen = nullptr;
        bool locked = false;
        std::function<void()> refresh_cb;
        std::function<void()> exit_cb;
    };

    NavigationState state_;
    ui::KindleMenu menu_;

    void populateMenu();
};
```

In `src/ui/screen_navigator.cpp`:
Implement:
```cpp
bool ScreenNavigator::isMenuOpen() const {
    return menu_.isOpen();
}

void ScreenNavigator::openMenu() {
    if (state_.locked) return;
    populateMenu();
}

void ScreenNavigator::closeMenu() {
    menu_.close();
}

void ScreenNavigator::populateMenu() {
    ui::MenuList list;
    if (state_.active_screen) {
        auto contextual = state_.active_screen->contextualMenuItems();
        for (const auto& item : contextual) {
            list.append(item);
        }
    }
    // Append global items with divider on first global item
    bool is_first_global = true;
    list.append(ui::MenuItem(ui::MenuLabel("Shortcuts & Help"), []() {}, is_first_global));
    list.append(ui::MenuItem(ui::MenuLabel("Refresh Screen"), [this]() {
        if (state_.refresh_cb) state_.refresh_cb();
    }));
    list.append(ui::MenuItem(ui::MenuLabel("Log Out"), [this]() {
        showLogin();
    }));
    list.append(ui::MenuItem(ui::MenuLabel("Exit Papergram"), [this]() {
        if (state_.exit_cb) state_.exit_cb();
    }));

    menu_.open(list);
}

void ScreenNavigator::render(Canvas& canvas) {
    if (state_.active_screen) state_.active_screen->render(canvas);
    if (menu_.isOpen()) menu_.render(canvas);
}

void ScreenNavigator::handleInput(const InputEvent& event) {
    if (event.pressed && event.code == KeyCode::KEY_MENU) {
        if (menu_.isOpen()) {
            menu_.close();
            return;
        }
        openMenu();
        return;
    }
    if (menu_.isOpen()) {
        menu_.handleInput(event);
        return;
    }
    if (state_.active_screen) state_.active_screen->handleInput(event);
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `make test`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/ui/screen_navigator.h src/ui/screen_navigator.cpp tests/test_screen_navigator_menu.cpp
git commit -m "feat(ui): integrate KindleMenu overlay and global actions into ScreenNavigator"
```

---

### Task 7: Full Test Suite, Sanitizers & Kindle Verification

**Files:**
- Test: All tests in `tests/`
- Build: `Makefile` targets (`client`, `test`, `test-asan`, `kindle`)

- [ ] **Step 1: Run complete unit test suite**

Run: `make test`
Expected: All tests pass with zero failures.

- [ ] **Step 2: Run AddressSanitizer and UndefinedBehaviorSanitizer suite**

Run: `make test-asan`
Expected: Zero memory leaks, zero buffer overflows, zero undefined behaviors.

- [ ] **Step 3: Build native client**

Run: `make client`
Expected: Successful build of `bin/kindle-telegram`.

- [ ] **Step 4: Cross-compile Kindle ARM32 binary if toolchain is present**

Run: `make kindle` (or verify cross-compilation target)
Expected: Clean compilation of `bin/kindle-telegram-arm32`.

- [ ] **Step 5: Final Git Status and Commit**

```bash
git status
```
Verify working tree is clean.
