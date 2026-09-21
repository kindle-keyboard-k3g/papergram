# Papergram 📖⚡

> **Distraction-free, serverless Telegram client crafted for the Amazon Kindle Keyboard (Kindle 3 / K3G / K3W) E-Ink display.**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Platform](https://img.shields.io/badge/Platform-Kindle%20Keyboard%20(K3)-black?logo=amazon)](https://en.wikipedia.org/wiki/Amazon_Kindle#Kindle_Keyboard)
[![Architecture](https://img.shields.io/badge/Arch-ARMv6%20%7C%20softfp-orange)](#hardware-specifications--compatibility)
[![Tests](https://img.shields.io/badge/Tests-33%20passed%20%7C%20ASan%20clean-brightgreen)](#quickstart--local-host-emulation)
[![Dependencies](https://img.shields.io/badge/Dependencies-Zero%20External-success)](#highlights--key-features)

---

## ASCII Screen & Interface

Papergram renders directly to the Kindle Keyboard 600×800 8bpp display buffer with word wrapping at 58 characters, message bubbles, and real-time status:

```text
+-------------------------------------------------------------------------+
| Papergram (Alice)                    12:34 PM   [98%]   [Wi-Fi]  [Back] |
+-------------------------------------------------------------------------+
|                                                                         |
|  +-------------------------------------------------------------+        |
|  | Alice:                                                      |        |
|  | Hey! Did you really get Telegram running on your Kindle     |        |
|  | Keyboard?!                                                  |        |
|  |                                                       12:30 |        |
|  +-------------------------------------------------------------+        |
|                                                                         |
|                    +--------------------------------------------------+ |
|                    | Yes! Zero servers, direct MTProto over Wi-Fi     | |
|                    | and 3G cellular. Fast DU partial e-ink updates!  | |
|                    |                                            12:32 | |
|                    +--------------------------------------------------+ |
|                                                                         |
|  +-------------------------------------------------------------+        |
|  | Alice:                                                      |        |
|  | That battery life must be incredible. Reading messages on   |        |
|  | E-Ink Pearl feels like reading an actual book!              |        |
|  |                                                       12:33 |        |
|  +-------------------------------------------------------------+        |
|                                                                         |
|                    +--------------------------------------------------+ |
|                    | Typing on the physical keyboard feels great.     | |
|                    |                                            12:34 | |
|                    +--------------------------------------------------+ |
|                                                                         |
+-------------------------------------------------------------------------+
| > Type a message..._                                          [Enter ↵] |
+-------------------------------------------------------------------------+
```

---

## Table of Contents

- [Highlights & Key Features](#highlights--key-features)
- [ASCII Screen & Interface](#ascii-screen--interface)
- [Hardware Specifications & Compatibility](#hardware-specifications--compatibility)
- [System Architecture](#system-architecture)
- [Prerequisites & Toolchain Setup](#prerequisites--toolchain-setup)
- [Quickstart & Local Host Emulation](#quickstart--local-host-emulation)
- [Kindle Deployment & Running](#kindle-deployment--running)
- [Physical Keyboard & Navigation Reference](#physical-keyboard--navigation-reference)
- [Troubleshooting & FAQ](#troubleshooting--faq)
- [Contributing & Coding Standards](#contributing--coding-standards)
- [License & Attribution](#license--attribution)

---

## Highlights & Key Features

- **On-Device & Serverless**: Runs 100% locally on the Kindle device. Connects directly to Telegram DC servers via MTProto RPC without any intermediate VPS bridge, companion daemon, or third-party relay.
- **Fast E-Ink Dual Refresh Engine**:
  - Direct Linux framebuffer access via `/dev/fb0` (600×800, 8bpp grayscale, 480 KB double buffer).
  - Hardware-accelerated DU (Direct Update) partial refreshes via `FBIO_EINK_UPDATE_DISPLAY_AREA` ioctl (`0x46dd`), delivering sub-50ms latency for typing and cursor blinking.
  - Automatic full GC16 flash refresh every 15 keystrokes or screen transitions to completely clear e-ink ghosting.
  - Dedicated ghostbuster shortcut: `Alt + G`.
- **Physical Keyboard & Input Multiplexer**:
  - Event-driven non-blocking `poll()` loop multiplexing Linux evdev inputs: `/dev/input/event0` (38-key physical QWERTY keyboard), `event1` (5-way directional D-pad), and `event2` (side page rocker buttons).
  - Smooth dialog navigation with selection wrapping and page scrolling.
- **Dual Network & Amazon 3G Proxy Support**:
  - Operates over standard Wi-Fi (`wlan0`) and Kindle 3G cellular (`wan0`/`ppp0`).
  - Integrated HTTP `CONNECT` tunneling designed for Amazon's cellular proxy (`fints.amazon.com:443`).
- **Zero-Dependency Micro-Crypto Engine**:
  - Self-contained implementations of AES-256 in IGE mode, SHA-1, SHA-256, and BigInteger modular exponentiation for Telegram Diffie-Hellman key exchange.
  - Embedded 8×16 monospace bitmap glyph font (ASCII + Latin-1) without external font engines (no FreeType, no HarfBuzz).
  - Strict resource budget: < 6 MB peak RAM, compiled binary size < 350 KB.
- **Strict Object Calisthenics & SOLID Architecture**:
  - 0 `else` keywords across the codebase: all conditional flow uses guard clauses, early returns, or polymorphism.
  - 1 indentation level per method; methods ≤ 15 lines; classes ≤ 100 lines; ≤ 2 instance fields per class.
  - Domain primitives wrapped in strong types (`PhoneNumber`, `AuthCode`, `ChatId`, `MessageId`, `MessageText`, `ScreenCoordinate`, `BoundingBox`, `GrayscaleColor`).
  - First-class collections (`ChatList`, `MessageHistory`, `DirtyRegionList`).

---

## Hardware Specifications & Compatibility

| Component | Kindle Keyboard (K3 / K3G / K3W) Reality | Papergram Architecture & Constraints |
| :--- | :--- | :--- |
| **Processor** | Freescale i.MX353, ARM1136JF-S @ 532 MHz | ARMv6TEJ (`-march=armv6j -mtune=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp`) |
| **Memory** | 256 MB Mobile DDR RAM (~100 MB free) | Peak RAM < 6 MB; launcher stops stock Java framework (`/etc/init.d/framework stop`) to free ~120 MB |
| **Display** | 6.0-inch E-Ink Pearl, 600×800, 16-level grayscale | In-memory 480 KB double buffer; direct `/dev/fb0` mapping; ioctl `0x46dd` DU partial / GC16 full |
| **Keyboard** | Physical 38-key QWERTY + 5-way D-pad + side rockers | Linux input events on `/dev/input/event[0,1,2]` multiplexed via non-blocking `poll()` |
| **Networking** | Wi-Fi 802.11 b/g + 3G Cellular (HSDPA/GSM) | MTProto over HTTP/HTTPS with Amazon proxy `CONNECT` tunneling (`fints.amazon.com:443`) |
| **Storage Layout** | 4 GB internal flash (/mnt/us user storage) | Binary & scripts in `/mnt/us/telegram/`; sessions persisted in `/mnt/us/telegram/session.dat` |
| **Supported Models** | K3G (D00901), K3W (B008), K3GB (B006), K3G UK (B00A) | Native binary compatible with glibc 2.5+ embedded on Kindle OS 3.x |

---

## System Architecture

```text
+-------------------------------------------------------------------------+
|                        Application Event Loop                           |
|      (poll multiplexing: Physical Keyboard + D-Pad + Timers)            |
+-------------------+---------------------------------+-------------------+
                    |                                 |
+-------------------v---------------+   +-------------v-------------------+
|             UI Layer              |   |          Domain Layer           |
| - ScreenNavigator (State Machine) |   | - Value Objects (Phone, Code)   |
| - LoginScreen (Phone, Code, 2FA)  |   | - ChatList & Chat               |
| - ChatListScreen (Dialog list)    |   | - MessageHistory & Message      |
| - ConversationScreen (Bubbles)    |   | - First-Class Collections       |
| - StatusHeader (Clock, Battery)   |   +---------------------------------+
+-------------------+---------------+
                    |
+-------------------v---------------+   +---------------------------------+
|         Graphics Engine           |   |       Telegram MTProto Core     |
| - Canvas (Double Buffer 600x800)  |   | - SessionStorage (AuthKey, Salt)|
| - BitmapFont (Embedded 8x16 font) |   | - Handshake (Diffie-Hellman)    |
| - DirtyTracker (Bounding Boxes)   |   | - TlCodec (Serialization)       |
| - EinkRefreshStrategy (DU / GC16) |   | - Crypto (AES-IGE, SHA256, DH)  |
+-------------------+---------------+   +----------------+----------------+
                    |                                    |
+-------------------v---------------+   +----------------v----------------+
|     Hardware Abstraction (HAL)    |   |         Network Layer           |
| - FrameBufferLinuxFb0 (/dev/fb0)  |   | - HttpTransport (MTProto HTTP)  |
| - EinkControllerMxc (ioctl 0x46dd)|   | - Amazon Proxy Tunnel (CONNECT) |
| - InputDeviceEvdev (/dev/input/ev)|   | - TelegramClient (RPC Dispatch) |
| - FallbackDevices (Host Emulation)|   +---------------------------------+
+-----------------------------------+
```

---

## Prerequisites & Toolchain Setup

### Host Prerequisites (Linux / macOS)

- **C++17 Compiler**: `g++` (≥ 9.0) or `clang++` (≥ 10.0)
- **Build System**: GNU `make`
- **Optional Sanitizers**: `libasan` and `libubsan` for development checks

### Cross-Compilation Toolchain for Kindle Keyboard

To build an ARM32 binary targeting the Kindle Keyboard ARMv6 architecture:

- **Debian / Ubuntu**:
  ```bash
  sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi
  ```
- **Kindle Tiny C Compiler**: You can also use `tcc` directly on the Kindle device using the `kindle-keyboard-k3g/kindle-tiny-c-compiler` distribution.

---

## Quickstart & Local Host Emulation

You do not need physical Kindle hardware to develop, test, or evaluate Papergram. When run on a PC, Papergram automatically detects the absence of `/dev/fb0` and falls back to:
- `MemoryFrameBuffer`: Simulates the 600×800 display in RAM and exports display snapshots to `/tmp/kindle_fb.ppm`.
- `DummyEinkController`: Records and logs e-ink partial and full refresh waveforms.
- `StdinInputDevice`: Translates standard terminal keystrokes into Kindle input events.

### 1. Build Native Host Executable

```bash
# Production release build (-O2, zero debug overhead)
make client
```

Binary will be produced at `bin/kindle-telegram`.

### 2. Debug Builds & Local Emulation

For active development, Papergram provides dedicated debug targets with unoptimized symbols (`-g3 -O0`) and conditional subsystem logging (`-DDEBUG`):

```bash
# Build native debug binary with GDB symbols and subsystem debug traces
make debug
# or: make client-debug

# Launch under GDB
gdb ./bin/kindle-telegram-debug

# Build and run native client with AddressSanitizer and UndefinedBehaviorSanitizer
make client-asan
./bin/kindle-telegram-asan

# Dynamically compile any target with debug flags
make client DEBUG=1
```

### 3. Run Locally on Host

```bash
./bin/kindle-telegram
```

You can view the exported framebuffer snapshot in any image viewer:

```bash
# View live rendered display output
xdg-open /tmp/kindle_fb.ppm
# or with ImageMagick:
display /tmp/kindle_fb.ppm
```

### 4. Run Unit Test Suite

Papergram includes a comprehensive zero-dependency test suite covering crypto primitives, canvas operations, e-ink refresh scheduling, domain value objects, and screen navigators:

```bash
# Run standard test suite
make test

# Run test suite with live subsystem debug traces
make test-debug

# Run test suite under AddressSanitizer & UBSan
make test-asan
```

---

## Kindle Deployment & Running

### 1. Kindle Preparation

1. Ensure your Kindle Keyboard has a jailbreak installed.
2. Install the **USBNetwork** package to enable SSH access over Wi-Fi or USB.
3. Test SSH connectivity:
   ```bash
   ssh root@<kindle-ip>
   ```

### 2. Cross-Compile & Deploy

```bash
# Cross-compile release binary for Kindle ARM32
make kindle

# Or cross-compile debug binary with symbols for Kindle gdbserver
make kindle-debug

# Create destination directory on Kindle user storage
ssh root@<kindle-ip> "mkdir -p /mnt/us/telegram"

# Copy binary and launcher script
scp bin/kindle-telegram-arm32 root@<kindle-ip>:/mnt/us/telegram/kindle-telegram
scp scripts/launch_kindle.sh root@<kindle-ip>:/mnt/us/telegram/launch_kindle.sh
ssh root@<kindle-ip> "chmod +x /mnt/us/telegram/kindle-telegram /mnt/us/telegram/launch_kindle.sh"
```

### 3. Launching on Device

Run the launcher script via SSH or a custom launch tile:

```bash
ssh root@<kindle-ip>
/mnt/us/telegram/launch_kindle.sh
```

**How the Launcher Works**:
- Automatically pauses the memory-heavy Kindle Java framework (`/etc/init.d/framework stop`), reclaiming ~120 MB RAM.
- Traps signals (`SIGINT`, `SIGTERM`, `EXIT`) to guarantee the stock Kindle framework is cleanly restarted (`/etc/init.d/framework start`) upon quitting.
- Redirects runtime logs to `/mnt/us/telegram/kindle-telegram.log`.

---

## Physical Keyboard & Navigation Reference

| Physical Key / Shortcut | Screen Context | Action |
| :--- | :--- | :--- |
| `D-Pad Up / Down` | Chat List | Move selection cursor up / down |
| `D-Pad Up / Down` | Conversation | Scroll message history up / down |
| `Enter` / `D-Pad Center` | Chat List | Open selected conversation |
| `Enter` / `D-Pad Center` | Conversation | Send typed message |
| `Back` Key | Conversation | Return to Chat List |
| `Next Page` Rocker (Right) | Conversation / List | Scroll down by a full screen page |
| `Prev Page` Rocker (Left) | Conversation / List | Scroll up by a full screen page |
| `Alt + G` | Any Screen | **Ghostbuster**: Force instant full GC16 e-ink flash refresh |
| `Alt + Q` | Any Screen | **Quit**: Cleanly terminate application and restore Kindle framework |

---

## Troubleshooting & FAQ

### Q: Does this require an external server or bridge daemon?
**A**: No. Papergram is 100% serverless and connects directly to official Telegram MTProto datacenters.

### Q: Why does the screen flash black occasionally?
**A**: E-Ink displays accumulate residual toner particles ("ghosting") when using fast partial updates. Papergram employs a dual refresh strategy: fast DU mode (<50ms) for typing and cursor movement, and an intentional full GC16 flash every 15 keystrokes or screen transitions to reset the particle pigments. You can trigger a manual flash at any time by pressing `Alt + G`.

### Q: Cellular 3G fails to connect through Amazon proxy
**A**: Ensure your Kindle 3G connection is active (`wan0` or `ppp0`). Kindle 3G proxies MTProto traffic through `fints.amazon.com:443` using standard HTTP `CONNECT` tunnels. If your cellular carrier blocks `CONNECT`, connect via Wi-Fi.

### Q: The Kindle Java interface did not restart after an unexpected shutdown
**A**: Connect to your Kindle via SSH and run:
```bash
/etc/init.d/framework start
lipc-set-prop com.lab126.powerd -i deferSuspend 1
```

### Q: Where is my Telegram session stored?
**A**: Auth keys and session salt are persisted in `/mnt/us/telegram/session.dat`. To switch accounts or log out, simply delete this file.

---

## Contributing & Coding Standards

Contributions and pull requests are welcome! To maintain software quality and reliability on constrained embedded hardware, this codebase strictly adheres to:

1. **Object Calisthenics**:
   - Only 1 level of indentation per method.
   - Do not use the `else` keyword (use guard clauses, early returns, or polymorphism).
   - Wrap all domain primitives in dedicated value classes.
   - First-class collections (collections must be wrapped in dedicated domain objects).
   - One dot per line (no chained method calls violating Demeter's Law).
   - No abbreviations in variable or class names.
   - Keep entities small: methods ≤ 15 lines, classes ≤ 100 lines, ≤ 2 fields per class.
   - No public getters or setters (tell, don't ask).
2. **Test-Driven Development (TDD)**:
   - Always run and verify tests before submitting changes:
     ```bash
     make test
     make test-asan
     ```

---

## License & Attribution

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

- **Author**: Samuel Caldas and contributors
- **Disclaimer**: *Telegram is a registered trademark of Telegram FZ-LLC. Papergram is an independent open-source project and is not affiliated with, sponsored by, or endorsed by Telegram FZ-LLC or Amazon.com, Inc.*
