# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Test Commands

- **Build native/host client**: `make client` (outputs `bin/kindle-telegram`)
- **Run all unit tests**: `make test` (builds and executes `bin/test_runner`)
- **Run tests with AddressSanitizer & UBSan**: `make test-asan`
- **Cross-compile for Kindle ARM32**: `make kindle` (outputs `bin/kindle-telegram-arm32`; requires `arm-linux-gnueabi-g++` or `kindle-tiny-c-compiler`)
- **Clean build artifacts**: `make clean`
- **Run a single test suite**: compile the specific test file alongside core sources and the test runner:
  ```bash
  g++ -std=c++17 -Wall -Wextra -pedantic -O2 -Isrc tests/test_runner.cpp tests/test_<name>.cpp $(filter-out src/main.cpp, $(wildcard src/*/*.cpp)) -o bin/test_single && ./bin/test_single
  ```
- **Run client locally on host**: `./bin/kindle-telegram` (automatically detects absence of `/dev/fb0` and falls back to `MemoryFrameBuffer` writing `/tmp/kindle_fb.ppm` and `StdinInputDevice`)

## Architecture & Code Structure

The client is a standalone, serverless C++17 Telegram client designed for the Kindle Keyboard (Kindle 3 / K3 ARM1136JF-S @ 532 MHz, 256 MB RAM) with zero third-party dependencies (no OpenSSL, Boost, or TDLib).

### Subsystem Flow

```
Input (/dev/input/event[0..2] or stdin) -> ScreenNavigator -> Active Screen -> Canvas (600x800 8bpp)
                                                                    |
                                                     DirtyTracker + RefreshStrategy
                                                                    |
                                           IFrameBuffer (/dev/fb0) + IEinkController (ioctl 0x46dd)
```

1. **Hardware Abstraction Layer (`src/hal/`)**:
   - `IFrameBuffer` / `FrameBufferLinuxFb0`: Memory-maps `/dev/fb0` (480 KB double-buffer at 600x800).
   - `IEinkController` / `EinkControllerMxc`: Controls Kindle i.MX35 e-ink updates via ioctl `FBIO_EINK_UPDATE_DISPLAY_AREA` (`0x46dd`) using DU mode (`fx_update_partial`) and GC16 mode (`fx_update_full`).
   - `IInputDevice` / `InputDeviceEvdev`: Multiplexes `/dev/input/event0` (keyboard), `event1` (D-pad), `event2` (page buttons).
   - `FallbackDevices` (`src/hal/fallback_devices.h`): `MemoryFrameBuffer`, `DummyEinkController`, and `StdinInputDevice` allow running and testing on non-Kindle development hosts.
   - `INetworkTransport` / `HttpTransport`: Handles HTTP POST and HTTP `CONNECT` proxy tunneling for both Wi-Fi and Kindle 3G cellular (via `fints.amazon.com:443`).

2. **Graphics Engine (`src/graphics/`)**:
   - `Canvas`: 600x800 8bpp double-buffered drawing surface with primitive drawing and PPM export.
   - `BitmapFont`: Monospace 8x16 embedded bitmap glyphs (ASCII + Latin-1) without external font engines.
   - `DirtyTracker`: Aggregates bounding boxes for partial e-ink updates.
   - `EinkRefreshStrategy`: `TypingRefresh` (fast DU partial updates) and `FullRefresh` (periodic GC16 flash every 15 keystrokes or screen transitions to clear ghosting).

3. **Telegram MTProto Core (`src/mtproto/`)**:
   - `crypto.h/.cpp`: Zero-dependency AES-256-IGE, SHA-1, SHA-256, and BigInteger modular exponentiation.
   - `handshake.h/.cpp`: Pollard's rho PQ factorization and Diffie-Hellman key exchange.
   - `tl_codec.h/.cpp`: TL serialization and deserialization primitives.
   - `session_storage.h/.cpp`: Persists authorization keys and session state to `/mnt/us/telegram/session.dat`.
   - `telegram_client.h/.cpp`: High-level MTProto RPC operations (`sendCode`, `signIn`, `checkPassword`, `getDialogs`, `getHistory`, `sendMessage`).

4. **UI Layer (`src/ui/`)**:
   - `ScreenNavigator`: State machine managing transitions between `LoginScreen`, `ChatListScreen`, and `ConversationScreen`.
   - `StatusHeader`: Clock, dynamic battery percentage, and network indicator (`[Wi-Fi]` / `[3G]`).
   - `LoginScreen`: On-device phone number (`+1...`), verification code, and 2FA password entry.
   - `ChatListScreen`: Dialog list with selection wrapping, unread badges, and page scrolling.
   - `ConversationScreen`: Message bubbles with 58-character word wrapping, page scrolling, and input bar.

5. **Device Deployment (`scripts/launch_kindle.sh`)**:
   - Halts the Kindle stock Java framework (`/etc/init.d/framework stop`) to free ~120 MB RAM, runs the client, and traps signals to restore the framework on exit.

## Coding Standards & Invariants

- **Object Calisthenics**:
  - Max 1 indentation level per method (extract helper methods).
  - No `else` keywords: use guard clauses, early returns, or polymorphism.
  - Wrap domain primitives (`PhoneNumber`, `AuthCode`, `ChatId`, `MessageId`, `MessageText`, `ScreenCoordinate`, `BoundingBox`, `GrayscaleColor`).
  - Use first-class collections (`ChatList`, `MessageHistory`, `DirtyRegionList`).
  - Keep entities small: classes <= 100 lines, methods <= 15 lines, <= 2 instance variables per class.
  - Tell, don't ask (expose behavior, not raw internal data).
- **Memory & Resource Budget**:
  - Target RAM footprint < 6 MB; binary size < 350 KB.
  - Avoid dynamic heap allocations in inner render loops.
