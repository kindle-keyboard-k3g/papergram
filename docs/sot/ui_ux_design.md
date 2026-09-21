# SOT: Modern Game Boy UI/UX Design System for Kindle E-Ink

> **Single Source of Truth (SOT) for Papergram's user interface, visual aesthetics, and interaction design.**

---

## 1. Vision & Design Philosophy

Papergram draws direct inspiration from the timeless aesthetic of the original 1989 Nintendo Game Boy (DMG-01) and reimagines it as a crisp, distraction-free, modern user experience tailored for the Amazon Kindle Keyboard's 6.0" E-Ink Pearl display (600×800, 8bpp/16-level grayscale).

### 1.1 The Core Concept: Retro Soul, Modern Mind

The Game Boy's UI was defined by absolute clarity under harsh ambient light, constrained display real estate, zero-latency physical D-pad controls, and high-contrast 2-bit (4-shade) graphics.

Papergram reinterprets these qualities for modern messaging:
- **Instant Optical Legibility**: Reflective E-Ink displays thrive on high contrast rather than subtle gradients or blur. The 4-shade palette produces razor-sharp visual hierarchy readable under direct sunlight or bedside lamplight.
- **Physical-First Ergonomics**: Designed exclusively for physical hardware controls—D-pad, Enter, Back, and side page rocker buttons—channeling the tactile satisfaction of handheld gaming consoles.
- **Modern Minimalism with Retro Warmth**: Chunky double-bordered dialogue boxes, card-style toasts, monospaced typography, and RPG-like speech bubbles coexist with clean modern margins, balanced whitespace, and word wrapping.
- **Zero-Ghosting Performance**: Visual elements are carefully chosen to minimize e-ink particle accumulation, maximizing the longevity of fast DU (Direct Update) partial refreshes before requiring a full GC16 flash.

---

## 2. Color Palette & Tone Hierarchy

Kindle E-Ink Pearl supports 16 levels of grayscale, but Papergram deliberately constrains its core UI elements to an authentic 4-tone palette, perfectly mirroring the Game Boy's 4-shade display while utilizing pure black and white for extreme contrast.

| Tone Name | Value Object | Hex / 8bpp Value | Visual Role | E-Ink Behavior |
| :--- | :--- | :--- | :--- | :--- |
| **White** | `GrayscaleColor::WHITE` | `0xFF` (255) | Canvas background, speech bubble fill | Native e-ink clear state; highly reflective |
| **Light Gray** | `GrayscaleColor::LIGHT_GRAY` | `0xC0` (192) | Selected chat row background, card fills, inactive badges | Fast settling time, excellent background contrast against black text |
| **Dark Gray** | `GrayscaleColor::DARK_GRAY` | `0x80` (128) | Inner card borders, timestamps, sender names, status indicators | Mid-tone separator; clean framing without visual heaviness |
| **Black** | `GrayscaleColor::BLACK` | `0x00` (0) | Primary typography, outer card borders, active focus indicators | Maximum contrast; high density particle alignment |

### 2.1 Contrast Rules
- **Text on White**: Must be `GrayscaleColor::BLACK` (primary text) or `GrayscaleColor::DARK_GRAY` (secondary metadata).
- **Text on Light Gray**: Must be `GrayscaleColor::BLACK` for maximum legibility.
- **Inverted Focus**: Highlighted elements (such as selected buttons or menu options) use `GrayscaleColor::BLACK` backgrounds with `GrayscaleColor::WHITE` text.
- **No Gradients or Dithering**: Gradients cause severe ghosting and slow down partial e-ink refreshes. All fills and backgrounds must be flat solid rectangles.

---

## 3. UI Component Architecture

```text
+-------------------------------------------------------------------------+
| [StatusHeader]                                                          |
| Papergram :: Alice                   12:34 PM   [98%]   [Wi-Fi]  [Back] |
+=========================================================================+
| [Conversation Area]                                                     |
|                                                                         |
|  +-------------------------------------------------------------+        |
|  | # Alice                                               12:30 |        |
|  | Hey! Did you really get Telegram running on your Kindle     |        |
|  | Keyboard?!                                                  |        |
|  +-------------------------------------------------------------+        |
|                                                                         |
|                    +--------------------------------------------------+ |
|                    | * You                                      12:32 | |
|                    | Yes! Zero servers, direct MTProto over Wi-Fi     | |
|                    | and 3G cellular. Fast DU partial e-ink updates!  | |
|                    +--------------------------------------------------+ |
|                                                                         |
|  +=============================================================+        |
|  || [ToastNotification: Double-line border card]              ||        |
|  || Keyboard Shortcut: Press Alt+G to clear e-ink ghosting    ||        |
|  +=============================================================+        |
|                                                                         |
+-------------------------------------------------------------------------+
| [InputBar]                                                              |
| > Type a message..._                                          [Enter ↵] |
+-------------------------------------------------------------------------+
```

### 3.1 Status Header
- **Inspiration**: Game Boy system status overlays and HUD headers.
- **Layout**: Top 24 pixels (`BoundingBox(0, 0, 600, 24)`).
- **Styling**: Solid white background, separated from the workspace by a crisp 1px black horizontal line.
- **Elements**:
  - Application title / Active context (`Papergram :: <Name>`) at `(10, 4)`.
  - Network state indicator in monospace brackets (`[Wi-Fi]`, `[3G]`, `[Offline]`).
  - Battery meter in monospace brackets (`[98%]`).
  - Digital clock (`12:34 PM`).

### 3.2 Double-Border Dialogue Cards (Toast & Popups)
- **Inspiration**: Classic RPG dialogue boxes (Pokémon, The Legend of Zelda: Link's Awakening).
- **Layout**: Floating centered card with dynamic width and height based on message content.
- **Styling**:
  - Outer border: 1px solid `GrayscaleColor::BLACK`.
  - Inner border: 1px solid `GrayscaleColor::DARK_GRAY` offset by 2 pixels inward.
  - Background fill: `GrayscaleColor::LIGHT_GRAY` (giving the card a distinctive tactile presence over the white canvas).
  - Title & text: Rendered in crisp `GrayscaleColor::BLACK` using safe bounding box clipping.

### 3.3 Chat List (Menu Screen)
- **Inspiration**: Game Boy start menus and inventory selection lists.
- **Layout**: 10 visible chat rows per page, 50px height per item.
- **Styling**:
  - Selected item: Solid `GrayscaleColor::LIGHT_GRAY` fill with dark gray bottom separator.
  - Unread badge: Monospace counter encased in brackets `[3]` positioned at the right margin.
  - Navigation: D-pad Up/Down shifts selection with auto-wrapping; Page Rockers or Left/Right jump by 5 items.

### 3.4 Conversation View (Message Screen)
- **Inspiration**: Handheld dialogue sequences with distinct speaker boxes.
- **Layout**:
  - Incoming messages: Left-aligned at `x = 20`, width = 440px.
  - Outgoing messages: Right-aligned at `x = 120`, width = 440px.
- **Styling**:
  - Border: 1px solid `GrayscaleColor::BLACK` rectangular bubble.
  - Header: Sender label in `GrayscaleColor::DARK_GRAY` with right-aligned timestamp.
  - Content: Wrapped text in `GrayscaleColor::BLACK`, 58-character limit per line.
  - Bottom input bar: Monospace prompt symbol `>` with cursor `_`.

---

## 4. Interaction Model & Physical Ergonomics

The Kindle Keyboard has no touchscreen; all interactions are physical, deliberate, and tactile.

| Physical Control | Game Boy Equivalent | Papergram Action | Tactile Response |
| :--- | :--- | :--- | :--- |
| **D-Pad Up / Down** | D-Pad ↑ / ↓ | Navigate chat items / scroll message history | Fast DU partial refresh |
| **D-Pad Center / Enter** | **A Button** | Select conversation / Send typed message | Instant action dispatch |
| **Back Key** | **B Button** | Exit conversation / Dismiss dialog / Cancel | Screen transition with GC16 clear |
| **Next / Prev Page Rockers** | **L / R Bumpers** | Page scroll (scroll 1 full page up or down) | Immediate block refresh |
| **Alt + G** | **Select + Start** | **Ghostbuster**: Manual full GC16 e-ink waveform flash | Full screen inversion & reset |
| **Alt + Q** | System Power Off | Quit client cleanly, restore Kindle OS framework | Terminal exit & cleanup |

---

## 5. E-Ink Waveform Synergy

Designing for E-Ink requires understanding physical pigment movement:
1. **DU (Direct Update)**: 2-tone black/white or fast 4-tone partial updates (<50ms). Used for:
   - Keystroke echo in the input bar.
   - Cursor blinking.
   - D-pad cursor movement in the chat list.
2. **GC16 (Grayscale 16 Clear)**: Multi-flash waveform (~300ms) that physically clears toner remnants. Used for:
   - Full screen navigation transitions.
   - Every 15 cumulative typing keystrokes to prevent residual ghosting.
   - Manual trigger via `Alt + G`.

By restricting the UI to solid rectangles, clean borders, and crisp bitmap glyphs without anti-aliasing fuzz, Papergram maximizes the clarity of DU mode and minimizes the visual need for intrusive GC16 flashes.
