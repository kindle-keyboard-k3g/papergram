# UI Architecture & Screen Flow

The UI subsystem (`src/ui/`) implements the visual presentation, navigation state machine, dialog boxes, and contextual menus. It adheres strictly to the **Modern Game Boy UI/UX Design System** ([`docs/sot/ui_ux_design.md`](../sot/ui_ux_design.md)) optimized for the Amazon Kindle Keyboard's E-Ink Pearl display.

---

## 1. Navigation & Screen Lifecycle

### `IScreen` (`src/ui/screen.h`)
Pure interface defining the lifecycle of each top-level application view:
- `void onEnter()`: Invoked when the screen becomes active; triggers initial data loading or cursor positioning.
- `void onExit()`: Invoked when leaving the screen; releases screen-specific resources.
- `void handleInput(const hal::InputEvent& event)`: Ingests user input.
- `void render(graphics::Canvas& canvas)`: Renders screen elements into the canvas back buffer.
- `std::vector<MenuItem> getContextMenuItems()`: Returns contextual actions displayed when the hardware `Menu` button is pressed.

### `ScreenNavigator` (`src/ui/screen_navigator.h`)
State machine managing screen transitions and modal overlays:
- **Registered Screens**:
  - `ScreenId::LOGIN`: `LoginScreen`
  - `ScreenId::CHAT_LIST`: `ChatListScreen`
  - `ScreenId::CONVERSATION`: `ConversationScreen`
  - `ScreenId::SCREENSAVER`: `ScreensaverScreen`
- **Modal Interceptors**:
  - `KindleMenu`: Hardware `Menu` button opens an overlay menu populated with global actions (Refresh, Lock, Exit) combined with contextual actions provided by the active screen.
  - `ConfirmationDialog`: Prompts user before destructive actions (e.g. exit application, logout).
- **Lock / Unlock State**: Pressing the power slider switches to `ScreensaverScreen`, saving previous state and halting active refresh timers.

```
       +---------------+
       |  LoginScreen  |
       +-------+-------+
               | (Authentication Success)
               v
       +---------------+  (Select Chat)  +--------------------+
       | ChatListScreen| <-------------> | ConversationScreen |
       +-------+-------+  (Back Button)  +--------------------+
               ^
               | (Power Slider / Timeout)
               v
       +---------------+
       |ScreensaverScrn|
       +---------------+
```

---

## 2. Screens

### `LoginScreen` (`src/ui/login_screen.h`)
3-step progressive authentication flow:
1. **Phone Number Step**: Prompts for international phone number (`+1...`). Renders a retro RPG hint card explaining that numbers are entered with physical number keys or Alt-chords.
2. **Verification Code Step**: Prompts for the 5-digit SMS/Telegram code.
3. **2FA Password Step**: Displayed only if the account has Two-Step Cloud Password enabled. Masks input text.

### `ChatListScreen` (`src/ui/chat_list_screen.h`)
Dialog overview:
- Lists up to 8 dialogs per page.
- Highlights current selection with an inverted high-contrast rectangle.
- Displays unread counter badge (`[3]`), contact/group title, and snippet of the latest message.
- Up/Down D-pad navigates items with wrap-around; Left/Right page rockers jump between pages; Enter opens conversation.

### `ConversationScreen` (`src/ui/conversation_screen.h`)
Interactive message thread:
- **Message Bubbles**: Renders incoming messages with light gray fill and outgoing messages on the right with white background and dark borders.
- **Word Wrapping**: Automatically breaks words cleanly at 58 characters (monospace 8x16 font fit across 600px width).
- **Input Bar**: Bottom bar for typing new messages. Supports typing, backspace, and Enter to send.

### `ScreensaverScreen` (`src/ui/screensaver_screen.h`)
Ultra-low power sleep display:
- Disables all active timers and periodic refreshes.
- Renders full-screen sleep artwork with status text ("Sleeping... Slide power switch to wake").
- Only wakes upon receiving `KEY_POWER`.

---

## 3. Menus & Modal Dialogs

### `KindleMenu` (`src/ui/kindle_menu.h`)
Double-bordered pop-up menu activated by the Kindle's physical `Menu` button:
- Dynamically combines **Global Menu Items**:
  - `Clear Ghosting (Alt+G)`: Forces immediate GC16 full refresh.
  - `Lock Device`: Enters screensaver immediately.
  - `Exit Papergram`: Opens `ConfirmationDialog`.
- With **Contextual Menu Items** supplied by the active screen:
  - Chat List: `Refresh Chats`, `New Chat`, `Search`.
  - Conversation: `Scroll to Top`, `View Info`, `Clear History`.
- Enforces single-focus trapping: while open, all navigation keys (`UP`, `DOWN`, `ENTER`, `BACK`) are swallowed by the menu.

### `ConfirmationDialog` (`src/ui/confirmation_dialog.h`)
Double-bordered modal prompt:
- Prompts user with "Are you sure you want to exit?" or similar destructive action queries.
- Offers `[Cancel]` (default safe choice) and `[Exit]` options.
- Left/Right arrows toggle focus; Enter triggers the selected callback.

---

## 4. Notifications & Popups

### `ToastNotification` (`src/ui/toast_notification.h`)
Retro RPG-inspired double-bordered dialogue card:
- Renders an outer 1px black border, a 2px inner dark gray border, and a light gray background fill.
- Renders 1 to 3 lines of centered monospace text.
- Used for action confirmations, network error warnings, and keyboard shortcut reminders.

### `PopupManager` (`src/ui/popup_manager.h`)
Unified notification coordinator:
- Manages positioning (`TOP`, `BOTTOM`, `CENTER`).
- Supports display modes:
  - `SEQUENTIAL_QUEUE`: Displays notifications one by one for a fixed duration.
  - `VISUAL_STACK`: Displays up to 4 concurrent cards stacked vertically.
- Automatically calculates damage areas and passes dirty bounding boxes to the graphics engine for partial e-ink refreshes.
