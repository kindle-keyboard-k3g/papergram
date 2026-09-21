# Implementation Plan: Tiny Telegram Client for Kindle Keyboard (Kindle 3 ARM32 E-Ink)

## 1. Context & Objectives

This project implements a lightweight, standalone, serverless Telegram client for the **Amazon Kindle Keyboard (Kindle 3 / K3)** e-ink device (ARM32 `armv6j`, 532 MHz, 256 MB RAM).
The client runs entirely on-device without requiring any external bridge server, relay, or cloud companion. A user can simply enter their phone number on the Kindle Keyboard, type the verification code received via SMS/Telegram, and begin reading and sending messages.

The project strictly follows:
- **Object Calisthenics**: Max 1 indent per method, no `else`, wrapped primitives, first-class collections, 1 dot per line, no abbreviations, <=2 fields per class, <=15 lines per method, <=100 lines per class, no public getters/setters.
- **SOLID Principles**: Single responsibility, open-closed, Liskov substitution, interface segregation, dependency inversion.
- **Test-Driven Development (TDD)**: Comprehensive unit tests with mocks for framebuffer, e-ink controller, input devices, and network transport before code implementation.
- **Target Hardware & Environment**: Direct Linux framebuffer `/dev/fb0`, ioctl `FBIO_EINK_UPDATE_DISPLAY_AREA` (`/dev/input/event0` keyboard, `event1` 5-way dpad, `event2` page keys), support for both Wi-Fi and 3G cellular (via Amazon HTTP CONNECT proxy), and build compatibility with `kindle-keyboard-k3g/kindle-tiny-c-compiler` (TCC) and ARM32 cross-toolchains.

---

## 2. Target Device Specifications & Constraints

| Hardware / Subsystem | Kindle Keyboard (K3) Reality | Client Design Solution |
| :--- | :--- | :--- |
| **CPU & Architecture** | Freescale i.MX353, ARM1136JF-S @ 532 MHz (`armv6j`, softfp, 32-bit little-endian) | Lightweight C++ (C++11/17 subset) with zero heavy frameworks. Binary size < 350 KB. |
| **RAM** | 256 MB total (~100 MB free) | Strict memory ceiling < 8 MB peak RAM. No dynamic heap fragmentation; static buffers & memory pools. |
| **Display (`/dev/fb0`)** | 600x800, 8bpp grayscale (or 4bpp packed). E-ink Pearl. | In-memory 480 KB double-buffer. Partial updates (DU mode) for typing & navigation; full GC16 flash every 15 updates or screen switch. |
| **Input Devices** | `/dev/input/event0` (keyboard), `/dev/input/event1` (d-pad), `/dev/input/event2` (volume/page buttons) | `epoll`/`poll` event loop multiplexing keyboard, dpad, and network sockets without busy-waiting. |
| **Networking** | Wi-Fi (`wlan0`) & 3G Cellular (`ppp0`/`wan0`) with Amazon proxy routing (`fints.amazon.com:443`) | MTProto encapsulated over HTTP/HTTPS with HTTP CONNECT proxy support. Works over Wi-Fi and 3G cellular. |
| **Toolchain** | `kindle-keyboard-k3g/kindle-tiny-c-compiler` & `arm-linux-gnueabi-gcc` | Clean modular C++ with minimal runtime dependencies (libc, libm). Custom micro-crypto (AES, SHA1, SHA256, DH). |

---

## 3. High-Level Architecture & Subsystem Decomposition

```
+-------------------------------------------------------------------------+
|                        Application Event Loop                           |
|      (epoll/poll multiplexing: Keyboard Inputs + Network + Timers)      |
+-------------------+---------------------------------+-------------------+
                    |                                 |
+-------------------v---------------+   +-------------v-------------------+
|             UI Layer              |   |          Domain Layer           |
| - ScreenNavigator                 |   | - UserAccount (Phone, AuthCode) |
| - LoginScreen (Phone, Code, 2FA)  |   | - ChatList & Chat               |
| - ChatListScreen (Dialogs)        |   | - MessageHistory & Message      |
| - ConversationScreen (Bubbles)    |   | - Value Objects (ChatId, etc.)  |
| - InputEditor (Text cursor)       |   +---------------------------------+
+-------------------+---------------+
                    |
+-------------------v---------------+   +---------------------------------+
|         Graphics Engine           |   |       Telegram MTProto Core     |
| - Canvas (Double Buffer 600x800)  |   | - MtprotoSession (AuthKey, Salt)|
| - BitmapFont (Embedded 8x16 font) |   | - Handshake (Diffie-Hellman)    |
| - DirtyTracker (Bounding Boxes)   |   | - TlSerializer & TlDeserializer |
| - EinkRefreshStrategy (DU / GC16) |   | - Crypto (AES-256-IGE, SHA256)  |
+-------------------+---------------+   +----------------+----------------+
                    |                                    |
+-------------------v---------------+   +----------------v----------------+
|     Hardware Abstraction (HAL)    |   |         Network Layer           |
| - FrameBufferLinuxFb0 (/dev/fb0)  |   | - HttpTransport (MTProto HTTP)  |
| - EinkControllerMxc (ioctl 0x46dd)|   | - AmazonProxyConnector (CONNECT)|
| - InputDeviceEvdev (/dev/input/ev)|   | - PollingWorker (getUpdates)    |
+-----------------------------------+   +---------------------------------+
```

---

## 4. Object Calisthenics & SOLID Implementation Strategy

### 4.1 Value Objects (Domain Primitives Wrapped)
No bare primitives (`int`, `std::string`, `char*`) will be passed across boundaries:
- `PhoneNumber`: Validates international E.164 format (e.g. `+1234567890`).
- `AuthCode`: Encapsulates 5-digit verification code.
- `CloudPassword`: Encapsulates 2FA password.
- `ChatId`: 64-bit Telegram peer identifier.
- `MessageId`: 32-bit unique message identifier.
- `MessageText`: Encapsulates UTF-8 string with length guards.
- `ScreenCoordinate`: Encapsulates `(x, y)` clamped to `0..600` and `0..800`.
- `BoundingBox`: Encapsulates `(left, top, right, bottom)` for partial e-ink refreshes.
- `GrayscaleColor`: Encapsulates 8-bit gray level (`BLACK = 0x00`, `DARK_GRAY = 0x55`, `LIGHT_GRAY = 0xAA`, `WHITE = 0xFF`).

### 4.2 First-Class Collections
Collections have their own dedicated classes exposing only collection behaviors:
- `ChatList`: Wraps `std::vector<Chat>`, exposes `selectNext()`, `selectPrevious()`, `selectedChat()`, `count()`.
- `MessageHistory`: Wraps `std::vector<Message>`, exposes `append()`, `visibleSlice(offset, count)`, `scrollUp()`, `scrollDown()`.
- `DirtyRegionList`: Wraps damaged rectangular regions and computes the minimal bounding box for e-ink partial updates.

### 4.3 Class Size & Structure Rules
- Classes are capped at <= 100 lines and <= 2 instance variables.
- When an entity requires more state, it is decomposed into composite collaborators (e.g., `ConversationView` delegates rendering to `MessageRenderer` and scrolling to `ScrollOffsetTracker`).
- Methods are capped at <= 15 lines and exactly 1 indentation level. Guard clauses and polymorphism replace `else`.
- Tell, Don't Ask: Instead of `if (chat.getUnreadCount() > 0)`, use `chat.renderUnreadBadgeTo(canvas)`.

---

## 5. Subsystems & Component Specifications

### 5.1 Hardware Abstraction Layer (HAL)
- `IFrameBuffer`:
  - `FrameBufferLinuxFb0`: Opens `/dev/fb0`, `mmap`s 480 KB (600x800x1 byte).
  - `MockFrameBuffer`: In-memory buffer for unit testing on x86_64 host, with PPM/PNG export for visual regression tests.
- `IEinkController`:
  - `EinkControllerMxc`: Issues `ioctl(fd, FBIO_EINK_UPDATE_DISPLAY_AREA, &update_area)` using `fx_update_partial` (0) for fast typing/navigation and `fx_update_full` (1) for full flashing refresh.
  - `MockEinkController`: Records refresh calls, waveforms, and bounding boxes for unit assertions.
- `IInputDevice`:
  - `InputDeviceEvdev`: Non-blocking read from `/dev/input/event0` (keyboard), `event1` (d-pad), `event2` (page buttons).
  - `MockInputDevice`: Injects synthetic key press/release events for automated TDD testing.
- `INetworkTransport`:
  - `HttpTransport`: Encapsulates HTTP/HTTPS POST to Telegram DC (`https://149.154.167.50:443/api`).
  - `AmazonProxyTransport`: Formats `CONNECT` requests for 3G cellular networks via `fints.amazon.com:443` or local proxy.
  - `MockNetworkTransport`: In-memory network mock returning scripted MTProto responses.

### 5.2 Graphics & Rendering Engine
- `Canvas`:
  - Dimensions: 600 width x 800 height, 8 bits per pixel (0 = black, 255 = white).
  - Primitives: `clear(color)`, `drawPixel(coord, color)`, `drawHorizontalLine(x, y, length, color)`, `drawVerticalLine(x, y, length, color)`, `drawRectangle(box, color)`, `fillRect(box, color)`, `invertRect(box)`.
- `BitmapFont`:
  - Embedded 8x16 monospace font (covers ASCII + Latin-1 + Telegram special symbols).
  - Glyph blitter renders directly to canvas with zero external library dependencies.
- `DirtyTracker`:
  - Accumulates dirty rectangles during drawing operations.
  - Merges overlapping bounding boxes to trigger a single partial `ioctl` per frame.
- `EinkRefreshStrategy`:
  - `TypingRefresh`: Triggers partial update (`fx_update_partial`) on bounding box of cursor / typed line (< 50ms latency).
  - `FullFlashRefresh`: Triggers full flashing GC16 update (`fx_update_full`) every 15 partial refreshes or on screen change.

### 5.3 Telegram MTProto & Authentication Engine
- `CryptoEngine`:
  - Standalone, zero-dependency implementations of:
    - AES-256 in IGE mode (`aes_ige_encrypt`, `aes_ige_decrypt`).
    - SHA-1 and SHA-256 message digests.
    - Minimal BigNum modular exponentiation for Diffie-Hellman key exchange (`g^a mod p`).
- `MtprotoHandshake`:
  - Implements Telegram DH exchange:
    1. `req_pq_multi` -> Receive `resPQ` (pq, server public key fingerprints).
    2. Factorize PQ using Pollard's rho algorithm.
    3. `req_DH_params` -> Receive encrypted server DH params.
    4. `set_client_DH_params` -> Compute auth_key (`(g_b)^a mod dh_prime`).
    5. Verify `server_salt` and derive `auth_key_id`.
- `SessionStorage`:
  - Persists `auth_key`, `server_salt`, `session_id`, and `dc_id` to `/mnt/us/telegram/session.dat`.
- `TelegramAuthService`:
  - `requestPhoneCode(PhoneNumber)` -> Calls `auth.sendCode`.
  - `submitAuthCode(AuthCode)` -> Calls `auth.signIn`.
  - `submitCloudPassword(CloudPassword)` -> Calls `auth.checkPassword`.
  - Auto-login on startup if valid session exists on disk.
- `TelegramChatService`:
  - `fetchDialogs()` -> Calls `messages.getDialogs` (updates `ChatList`).
  - `fetchHistory(ChatId)` -> Calls `messages.getHistory` (updates `MessageHistory`).
  - `sendMessage(ChatId, MessageText)` -> Calls `messages.sendMessage`.
  - `pollUpdates()` -> Calls `updates.getDifference` or long-polling.

### 5.4 UI Screens & Navigation
- `ScreenNavigator`:
  - Manages active screen: `LoginScreen` -> `ChatListScreen` -> `ConversationScreen` -> `SettingsScreen`.
- `LoginScreen`:
  - Step 1: "Enter Phone Number: [+1234567890]" with instructions for Kindle physical keyboard.
  - Step 2: "Enter Verification Code: [12345]".
  - Step 3 (if 2FA enabled): "Enter Cloud Password: [******]".
  - Displays clear error notifications (invalid code, rate limit, proxy error).
- `ChatListScreen`:
  - Status header: Time, Battery %, Wi-Fi/3G indicator, Unread count.
  - Scrollable list of chats: Title, last message preview, unread count pill.
  - D-pad Up/Down selects chat; Enter or Right opens conversation.
  - Next Page / Prev Page rocker keys scroll 8 chats at a time.
- `ConversationScreen`:
  - Header: Chat Title, online status, Back indicator (`[Back]`).
  - Message bubble list: Sender name, text (with word wrapping at 58 chars), timestamp, incoming vs outgoing alignment.
  - Bottom input bar: "Type a message..." with active cursor.
  - Typing triggers instant partial e-ink refresh on the input box.
  - Enter sends the message; Back key returns to `ChatListScreen`.

---

## 6. Critical Files to Create

```
cpp-kindle-telegram-client/
├── Makefile                          # Cross-compilation & host test build rules
├── src/
│   ├── main.cpp                      # Application entry point & epoll event loop
│   ├── hal/
│   │   ├── frame_buffer.h            # IFrameBuffer interface
│   │   ├── frame_buffer_fb0.cpp      # Linux /dev/fb0 implementation
│   │   ├── eink_controller.h         # IEinkController interface & ioctls
│   │   ├── eink_controller_mxc.cpp   # Kindle i.MX35 ioctl implementation
│   │   ├── input_device.h            # IInputDevice interface
│   │   ├── input_device_evdev.cpp    # /dev/input/event[0,1,2] reader
│   │   ├── network_transport.h       # INetworkTransport interface
│   │   └── http_transport.cpp        # Telegram HTTP/HTTPS & Amazon proxy
│   ├── graphics/
│   │   ├── canvas.h / .cpp           # 600x800 8bpp drawing canvas
│   │   ├── bitmap_font.h / .cpp      # Monospace 8x16 embedded bitmap font
│   │   ├── dirty_tracker.h / .cpp    # Bounding box accumulator
│   │   └── refresh_strategy.h / .cpp # DU partial vs GC16 full refresh
│   ├── domain/
│   │   ├── value_objects.h / .cpp    # PhoneNumber, AuthCode, ChatId, etc.
│   │   ├── chat.h / .cpp             # Chat entity & ChatList collection
│   │   └── message.h / .cpp          # Message entity & MessageHistory collection
│   ├── mtproto/
│   │   ├── crypto.h / .cpp           # Standalone AES-256-IGE, SHA1, SHA256, BigNum
│   │   ├── handshake.h / .cpp        # DH key exchange & session generation
│   │   ├── session_storage.h / .cpp  # Local session persistence (/mnt/us/telegram)
│   │   ├── tl_codec.h / .cpp         # TL schema serializer/deserializer
│   │   └── telegram_client.h / .cpp  # High-level Telegram client service
│   └── ui/
│       ├── screen_navigator.h / .cpp # Screen state machine
│       ├── login_screen.h / .cpp     # Phone number, code, and 2FA screen
│       ├── chat_list_screen.h / .cpp # Dialogs list view
│       ├── conversation_screen.h/.cpp# Message stream & input editor
│       └── status_header.h / .cpp    # Battery, clock, network status
└── tests/
    ├── test_runner.cpp               # Lightweight zero-dependency test runner
    ├── mocks/
    │   ├── mock_frame_buffer.h       # In-memory canvas with PPM export
    │   ├── mock_eink_controller.h    # E-ink ioctl recorder
    │   ├── mock_input_device.h       # Synthetic keyboard event injector
    │   └── mock_network_transport.h  # Scripted MTProto network responses
    ├── test_value_objects.cpp        # Unit tests for domain primitives
    ├── test_canvas.cpp               # Unit tests for drawing & text rendering
    ├── test_eink_strategy.cpp        # Unit tests for partial/full refresh cycles
    ├── test_crypto.cpp               # Unit tests for AES-IGE, SHA256, BigNum
    ├── test_handshake.cpp            # Unit tests for MTProto DH exchange
    ├── test_chat_list.cpp            # Unit tests for chat navigation
    └── test_conversation.cpp         # Unit tests for message rendering & typing
```

---

## 7. Edge Cases & Resilience Strategy

1. **3G Cellular Drops & Amazon Proxy Timeouts**:
   - Connection retry with exponential backoff (`1s`, `2s`, `4s`, `8s`, max `30s`).
   - Network status banner displayed non-intrusively in the header (`Offline / Reconnecting...`).
   - Outgoing messages queued locally in `PendingMessageQueue`; automatically dispatched when connection is restored.
2. **E-Ink Ghosting**:
   - Automatic full GC16 flash refresh triggered after every 15 partial updates, or immediately upon switching chats.
   - User shortcut: Pressing `Alt + G` forces an instant full screen clear and redraw.
3. **Kindle Framework Coexistence & Suspend/Resume**:
   - Launcher shell script stops stock Kindle Java UI (`/etc/init.d/framework stop`) to free ~120 MB RAM, and restarts it on exit (`/etc/init.d/framework start`).
   - Handles `SIGTERM` and `SIGINT` cleanly: releases `/dev/fb0`, unmaps memory, and flushes session data.
4. **Physical Keyboard Shortcuts**:
   - `Enter`: Send message / Select chat.
   - `Back` key: Go back to Chat List.
   - `Next Page / Prev Page` side buttons: Scroll messages or chats by page.
   - `Alt + Space`: Insert symbol / emoji picker.
   - `Alt + Q`: Exit application.

---

## 8. Verification & TDD Execution Plan

1. **Host-Side Unit Tests (`make test`)**:
   - Compile and execute all unit tests on host x86_64 using `g++ -std=c++17 -Wall -Wextra -pedantic`.
   - Verify 100% pass rate for:
     - `test_value_objects`: validation of phone numbers, codes, bounds.
     - `test_crypto`: test vectors for AES-256-IGE, SHA1, SHA256, DH exchange.
     - `test_canvas`: pixel accuracy, text wrapping, bounds clipping.
     - `test_eink_strategy`: verify partial vs full refresh triggers.
     - `test_conversation`: verify typing partial refresh bounding boxes.
2. **Visual Framebuffer Dump Verification**:
   - `MockFrameBuffer` exports generated frames to PPM images (`test_chat_screen.ppm`, `test_conversation.ppm`).
   - Verify layout, contrast, and readability of bubbles and text at 600x800 resolution.
3. **Cross-Compilation (`make kindle`)**:
   - Cross-compile using ARM toolchain (`arm-linux-gnueabi-g++` or `kindle-tiny-c-compiler`).
   - Verify binary is statically or dynamically linked against Kindle glibc, size <= 350 KB, zero missing symbols.
