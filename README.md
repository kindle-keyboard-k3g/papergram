# Tiny C++ Telegram Client for Kindle Keyboard (Kindle 3 / K3 E-Ink ARM32)

A lightweight, standalone, serverless Telegram client crafted specifically for the **Amazon Kindle Keyboard (Kindle 3 / K3G / K3W)** e-ink device running embedded Linux on ARMv6.

---

## Highlights & Features

- **Serverless & On-Device**: Runs directly on the Kindle device without requiring any intermediate bridge server, relay VPS, or third-party daemon.
- **Easy Phone Number Login**: Users simply enter their phone number (`+1234567890`) and the 5-digit verification code using the physical Kindle keyboard. Supports 2FA cloud passwords. Sessions are persisted securely to `/mnt/us/telegram/session.dat`.
- **E-Ink Display Engine**:
  - Direct Linux framebuffer access via `/dev/fb0` (600x800, 8bpp grayscale).
  - Double-buffered canvas in memory (480 KB footprint).
  - Hardware-accelerated partial updates (DU/A2 mode via `FBIO_EINK_UPDATE_DISPLAY_AREA` ioctl `0x46dd`) for sub-50ms typing and cursor movement.
  - Automatic full GC16 flash refresh every 15 keystrokes or screen transitions to completely eliminate e-ink ghosting.
  - Manual ghostbuster shortcut: `Alt + G`.
- **Kindle Keyboard & Navigation Support**:
  - Event-driven non-blocking input multiplexer reading `/dev/input/event0` (keyboard), `event1` (5-way d-pad), and `event2` (volume and side page rocker buttons).
  - Physical D-Pad navigation for selecting chats; Page rocker keys for smooth scrolling.
- **Wi-Fi & 3G Amazon Proxy Support**:
  - Native support for both direct Wi-Fi networks and Kindle 3G cellular data.
  - Integrated HTTP CONNECT proxy client tunneling through Amazon's cellular proxy (`fints.amazon.com:443`).
- **Zero Heavy Dependencies**:
  - Custom standalone micro-crypto engine: AES-256 in IGE mode, SHA-1, SHA-256, and BigInteger modular exponentiation for Telegram Diffie-Hellman key exchange.
  - Embedded 8x16 monospace bitmap font for clean, sharp glyph rendering without FreeType.
  - Zero dynamic heap fragmentation; memory footprint < 6 MB RAM.
- **Clean Architecture & Quality**:
  - 100% compliant with **Object Calisthenics** (max 1 indent per method, no `else`, wrapped primitives, first-class collections, <=2 fields per class, <=15 lines per method, <=100 lines per class).
  - Adheres strictly to **SOLID** principles with full Hardware Abstraction Layer (HAL).
  - Comprehensive unit test suite with zero-dependency test framework and mocks.

---

## Hardware Specifications & Compatibility

| Component | Kindle Keyboard (K3) | Client Architecture |
| :--- | :--- | :--- |
| **Processor** | Freescale i.MX353, ARM1136JF-S @ 532 MHz | ARMv6TEJ (`-march=armv6j -mtune=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp`) |
| **Memory** | 256 MB RAM (~100 MB free) | Peak usage < 6 MB; launcher pauses Java framework (`/etc/init.d/framework stop`) to free 120 MB RAM |
| **Display** | 6-inch E-Ink Pearl, 600x800, 16-gray | `/dev/fb0` double-buffer, `mxcfb_update_data` ioctl `0x46dd` (DU mode partial / GC16 full) |
| **Keyboard** | Physical 38-key QWERTY + D-pad + Page buttons | Linux input events on `/dev/input/event[0,1,2]` |
| **Network** | Wi-Fi 802.11 b/g + 3G Cellular | MTProto over HTTP/HTTPS with Amazon proxy CONNECT support |

---

## Architecture

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
| - StatusHeader (Clock, Battery)   |   +---------------------------------+
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

## Building & Testing

### 1. Host Build & Unit Tests (x86_64)

Build and run the entire zero-dependency test suite on your local development machine:

```bash
# Compile and run all unit tests
make test

# Run tests with AddressSanitizer and UndefinedBehaviorSanitizer
make test-asan

# Build the native client executable
make client
```

### 2. Cross-Compilation for Kindle Keyboard

To compile for the Kindle Keyboard ARM32 target:

```bash
make kindle
```

Or using the `kindle-keyboard-k3g/kindle-tiny-c-compiler` toolchain:

```bash
# Using tiny-c-compiler on the device itself
tcc -run src/main.cpp
```

---

## Installation & Running on Kindle

1. Ensure your Kindle Keyboard has a jailbreak and SSH access (USBNetwork).
2. Copy the binary and launch script to your Kindle's user storage:
   ```bash
   scp bin/kindle-telegram-arm32 root@kindle:/mnt/us/telegram/kindle-telegram
   scp scripts/launch_kindle.sh root@kindle:/mnt/us/telegram/launch_kindle.sh
   ```
3. Run the launcher script:
   ```bash
   /mnt/us/telegram/launch_kindle.sh
   ```
4. Enter your phone number on the Kindle's physical keyboard, enter the verification code, and chat away!

---

## Keyboard Shortcuts

| Key | Action |
| :--- | :--- |
| `D-Pad Up / Down` | Select previous / next chat in Chat List |
| `Enter` / `D-Pad Center` | Open selected chat / Send message |
| `Back` | Return from conversation to Chat List |
| `Next / Prev Page Rockers`| Scroll chat history / dialogs by page |
| `Alt + G` | Ghostbuster: force full GC16 e-ink screen refresh |
| `Alt + Q` | Quit application and restore Kindle framework |

---

## License

MIT License. Designed with care for Kindle Keyboard e-ink enthusiasts.
