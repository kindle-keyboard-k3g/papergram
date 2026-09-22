# Architecture Overview

Papergram is a standalone, serverless C++17 Telegram client designed specifically for the Amazon Kindle Keyboard (Kindle 3 / K3 ARM1136JF-S @ 532 MHz, 256 MB RAM) operating under strict embedded resource constraints:
- **Zero third-party library dependencies**: No OpenSSL, Boost, libcurl, or TDLib. All cryptography, TL serialization, networking, and font rendering are implemented natively.
- **Resource budget**: RAM footprint < 6 MB; binary size < 350 KB.
- **Display**: 600x800 8-bit grayscale E-Ink Pearl display driven directly via Linux framebuffer (`/dev/fb0`) and custom ioctl calls (`0x46dd`).
- **Input**: Evdev event multiplexing (`/dev/input/event0` keyboard, `event1` D-pad, `event2` page rockers).

---

## 1. System Pipeline

```
+-------------------------------------------------------------------------+
|                              Input Event                                |
|          (/dev/input/event[0..2], /dev/input/event0, or stdin)          |
+------------------------------------+------------------------------------+
                                     |
                                     v
+-------------------------------------------------------------------------+
|                            IInputDevice                                 |
|         (Translates raw evdev scancodes into high-level KeyEvents)       |
+------------------------------------+------------------------------------+
                                     |
                                     v
+-------------------------------------------------------------------------+
|                           ScreenNavigator                               |
|        (Routes keys to KindleMenu / ConfirmationDialog / ActiveScreen)  |
+------------------------------------+------------------------------------+
                                     |
                                     v
+-------------------------------------------------------------------------+
|                        Active IScreen / Overlay                         |
|     (LoginScreen, ChatListScreen, ConversationScreen, Screensaver)      |
+------------------------------------+------------------------------------+
                                     |  Renders primitives
                                     v
+-------------------------------------------------------------------------+
|                             Canvas                                      |
|            (600x800 8bpp double-buffered drawing surface)               |
+------------------------------------+------------------------------------+
                                     |
                                     v
+------------------------------------+------------------------------------+
|               DirtyTracker / BufferDiffTracker                          |
|         (Tracks modified pixel regions & dark-to-white transitions)      |
+------------------------------------+------------------------------------+
                                     |
                                     v
+------------------------------------+------------------------------------+
|                         RefreshStrategy                                 |
|          (TypingRefresh DU mode, FullRefresh GC16 flash, IdleRefresh)   |
+------------------------------------+------------------------------------+
                                     |
                                     v
+------------------------------------+------------------------------------+
|                    IEinkController + IFrameBuffer                       |
|           (/dev/fb0 mmap + FBIO_EINK_UPDATE_DISPLAY_AREA ioctl)         |
+-------------------------------------------------------------------------+
```

---

## 2. Layered Subsystems

The codebase is organized into clean, decoupled layers adhering to SOLID principles and Object Calisthenics:

```
src/
├── main.cpp                 # Application entrypoint & main event loop
├── domain/                  # Pure domain primitives and entities
│   ├── value_objects.h/.cpp # PhoneNumber, AuthCode, ChatId, MessageId, BoundingBox
│   ├── chat.h/.cpp          # Chat entity and ChatList collection
│   └── message.h/.cpp       # Message entity and MessageHistory collection
├── graphics/                # E-Ink graphics pipeline and renderers
│   ├── canvas.h/.cpp        # 600x800 8bpp double-buffered canvas
│   ├── bitmap_font.h/.cpp   # Embedded monospace 8x16 bitmap font
│   ├── dirty_tracker.h/.cpp # Bounding box aggregation for partial updates
│   ├── diff_tracker.h/.cpp  # Front vs back buffer diff calculation
│   ├── dark_to_white_cleaner.h/.cpp # Ghosting mitigation on transitions
│   ├── idle_refresh_scheduler.h/.cpp # Background 4x4 checkerboard GC16 sweep
│   └── refresh_strategy.h/.cpp # DU partial vs GC16 full refresh state machine
├── hal/                     # Hardware Abstraction Layer
│   ├── frame_buffer.h       # Framebuffer interface
│   ├── frame_buffer_fb0.h/.cpp # Linux /dev/fb0 mmap implementation
│   ├── eink_controller.h    # E-Ink controller interface
│   ├── eink_controller_mxc.h/.cpp # Freescale i.MX35 EPDC ioctl driver
│   ├── input_device.h       # Input device interface & key constants
│   ├── input_device_evdev.h/.cpp # Linux /dev/input event reader
│   ├── fallback_devices.h/.cpp # Host simulation (PPM output, dummy eink, stdin)
│   ├── network_transport.h  # Raw TCP/HTTP transport interface
│   ├── http_transport.h/.cpp # Wi-Fi and 3G HTTP tunneling transport
│   └── async_worker.h/.cpp  # Dedicated background thread with UI queue
├── mtproto/                 # MTProto 2.0 Telegram protocol engine
│   ├── crypto.h/.cpp        # AES-256-IGE, SHA-1, SHA-256, BigInteger DH
│   ├── handshake.h/.cpp     # Pollard's rho PQ factorization & DH exchange
│   ├── tl_codec.h/.cpp      # Type Language binary serialization primitives
│   ├── session_storage.h/.cpp # Session persistence (/mnt/us/telegram/session.dat)
│   └── telegram_client.h/.cpp # High-level RPC methods (auth, dialogs, messages)
├── ui/                      # Game Boy aesthetic user interface
│   ├── screen.h             # IScreen lifecycle & contextual menu provider
│   ├── screen_navigator.h/.cpp # Screen state machine & modal overlay manager
│   ├── status_header.h/.cpp # Battery, time, and network status bar
│   ├── screensaver_screen.h/.cpp # Low-power lock screen with sleep art
│   ├── login_screen.h/.cpp  # Multi-step phone, code, and 2FA authentication
│   ├── chat_list_screen.h/.cpp # Paginated dialog overview
│   ├── conversation_screen.h/.cpp # Paginated message history & text entry
│   ├── toast_notification.h/.cpp # Retro RPG double-bordered feedback cards
│   ├── kindle_menu.h/.cpp   # Top-level Kindle Menu button overlay
│   ├── menu_item.h/.cpp     # Menu action descriptors
│   ├── menu_label.h/.cpp    # Validated menu labels
│   ├── menu_list.h/.cpp     # First-class menu item collection with wrap-around
│   ├── confirmation_dialog.h/.cpp # Modal confirmation prompt (e.g., Exit)
│   ├── popup_types.h/.cpp   # Popup positions, display modes, timestamps
│   ├── popup_entry.h/.cpp   # Active popup timer and state tracking
│   ├── popup_layout.h/.cpp  # Coordinate layout calculation for popups
│   ├── popup_collection.h/.cpp # FIFO visual stack and sequential queue
│   └── popup_manager.h/.cpp # Unified toast and popup orchestrator
└── util/                    # Shared utilities
    └── debug_log.h          # Conditional compile-time debug tracing
```

---

## 3. Concurrency & Event Loop Model

Papergram operates on an **event-driven single-threaded UI loop** augmented with a dedicated **background worker thread** (`AsyncWorker`):

1. **Non-blocking input polling**: Input events are read from the HAL with a short timeout.
2. **UI callback draining**: Any asynchronous operations (MTProto network RPCs, handshake, dialog fetching) running on `AsyncWorker` post their completion lambdas into a thread-safe queue. The main loop drains these callbacks at the top of every iteration, updating UI state safely on the main thread without mutex locks on UI widgets.
3. **Screen rendering**: The active screen renders its dirty elements to the `Canvas` back buffer.
4. **Buffer diff & partial update**: `BufferDiffTracker` computes the exact rectangular union of changed pixels between `frontBuffer` and `backBuffer`.
5. **E-ink hardware dispatch**: `EinkRefreshStrategy` issues either a fast partial waveform update (`DU`) for immediate feedback or a full refresh (`GC16`) to purge ghosting.
6. **Idle maintenance**: When the user is inactive for >5 seconds, `IdleRefreshScheduler` runs an interleaved 4x4 checkerboard background refresh (16 tiles of 150x200 px spaced 400ms apart) that instantly preempts upon any keypress.
