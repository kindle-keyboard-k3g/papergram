# Implementation Plan: Papergram README Overhaul (GitHub Best Practices & MIT License)

## 1. Context & Objectives

The project implements a lightweight, standalone, serverless Telegram client for the Amazon Kindle Keyboard (Kindle 3 / K3 ARMv6 E-Ink). Following the user's decision, the project will be commercially branded as **Papergram** (*The E-Ink Telegram Client for Kindle Keyboard*), while retaining the compiled binary and script names (`bin/kindle-telegram`) for backward compatibility.

Currently:
- `README.md` has good technical content but lacks modern GitHub presentation (commercial branding, GitHub shields/badges, Table of Contents, ASCII UI screen mockup, explicit host emulation steps, troubleshooting guide, and contribution instructions).
- The repository is missing an official standalone `LICENSE` file in the root directory.

The objective is to establish a polished, professional GitHub repository presentation adhering to modern GitHub README standards and providing the official MIT License.

---

## 2. Proposed Changes & Structure

### 2.1 Create Root `LICENSE` File
- Standard MIT License text.
- Copyright (c) 2026 Samuel Caldas and contributors.

### 2.2 Modernize and Expand `README.md`
The new `README.md` will be structured as follows:

1. **Header & Badges**:
   - Project Brand: `# Papergram 📖⚡`
   - Subtitle: *Distraction-free, serverless Telegram client crafted for the Amazon Kindle Keyboard (Kindle 3 / K3G / K3W) E-Ink display.*
   - Shields/Badges:
     - `License: MIT` -> links to `LICENSE`
     - `C++ Standard: C++17`
     - `Platform: Kindle Keyboard (K3)`
     - `Architecture: ARMv6 | softfp`
     - `Tests: 33 passed | ASan / UBSan clean`
     - `Dependencies: Zero`

2. **ASCII Screen & UI Mockup**:
   - High-fidelity ASCII rendering of the Kindle 600x800 display showing:
     - Top Status Bar (Clock `12:34`, Battery `[98%]`, Network `[Wi-Fi]` / `[3G]`)
     - Message Bubbles (incoming left-aligned with sender, outgoing right-aligned, 58-character word wrapping)
     - Bottom Input Bar (`Type a message..._`) with active cursor

3. **Table of Contents**:
   - Direct markdown anchor links to all sections.

4. **Key Features**:
   - **On-Device & Serverless**: Direct MTProto connection to Telegram DCs; no relay VPS or third-party bridge.
   - **Fast E-Ink Dual Refresh**: Hardware ioctl `0x46dd` using DU partial mode (<50ms typing latency) + GC16 full flash anti-ghosting.
   - **Hardware Input Multiplexer**: Non-blocking `poll()` reading physical QWERTY keyboard (`event0`), 5-way D-pad (`event1`), and side page rockers (`event2`).
   - **Dual Network & Amazon Proxy**: Works seamlessly over Wi-Fi and 3G cellular via HTTP CONNECT tunneling (`fints.amazon.com:443`).
   - **Zero-Dependency Micro-Crypto**: Standalone AES-256-IGE, SHA-1, SHA-256, and BigInteger modular exponentiation.
   - **Strict Object Calisthenics & SOLID**: Clean architecture with 0 `else` branches, wrapped domain primitives, and first-class collections.

5. **Hardware Specifications & Compatibility Matrix**:
   - Detailed specifications table covering Freescale i.MX353 ARM1136JF-S @ 532 MHz, 256 MB RAM, E-Ink Pearl 600x800 8bpp display, physical inputs, and storage layout.

6. **System Architecture**:
   - Updated ASCII architecture diagram illustrating the interaction between the Hardware Abstraction Layer (`IFrameBuffer`, `IEinkController`, `IInputDevice`, `INetworkTransport`), Graphics Engine (`Canvas`, `BitmapFont`, `DirtyTracker`, `RefreshStrategy`), MTProto Core (`Crypto`, `Handshake`, `SessionStorage`), and UI Screens (`ScreenNavigator`, `LoginScreen`, `ChatListScreen`, `ConversationScreen`).

7. **Prerequisites & Toolchain Setup**:
   - Host prerequisites: C++17 compiler (`g++` or `clang++`), GNU Make.
   - Cross-compilation: `arm-linux-gnueabi-g++` or `kindle-tiny-c-compiler` (TCC).

8. **Quickstart & Local Host Emulation**:
   - Step-by-step instructions to compile and test on host PC:
     - `make client`
     - Running `./bin/kindle-telegram` with automatic fallback to `MemoryFrameBuffer` writing `/tmp/kindle_fb.ppm` and `StdinInputDevice`.
     - `make test` (33 unit tests)
     - `make test-asan` (AddressSanitizer and UndefinedBehaviorSanitizer)

9. **Kindle Deployment & Running**:
   - Jailbreak & USBNetwork (SSH) setup.
   - Deploying binary and `scripts/launch_kindle.sh` to `/mnt/us/telegram/`.
   - Running launcher script (automatically stops stock Java framework to free 120 MB RAM, traps signals, and restores framework on exit).

10. **Physical Keyboard & Navigation Reference**:
    - Comprehensive table of physical keys and shortcuts:
      - D-Pad navigation (select chats, scroll)
      - Enter / Center D-Pad (open chat, submit message)
      - Back key (return to chat list)
      - Page rocker buttons (page up/down in conversation and dialog list)
      - `Alt + G` (Ghostbuster: force full GC16 screen refresh)
      - `Alt + Q` (Clean quit and framework restoration)

11. **Troubleshooting & FAQ**:
    - Cellular 3G proxy errors and reconnect behavior.
    - E-ink ghosting mitigation.
    - Framework recovery if application terminated unexpectedly.

12. **Contributing**:
    - Guidance on TDD workflow, running `make test` and `make test-asan`.
    - Object Calisthenics rules (0 `else`, classes <= 100 lines, methods <= 15 lines, wrapped primitives).

13. **License & Attribution**:
    - MIT License summary with link to `LICENSE`.
    - Copyright notice.

---

## 3. Critical Files to Create / Modify

- `LICENSE`: Create standard MIT License file.
- `README.md`: Rewrite and expand to incorporate the Papergram branding, badges, ASCII UI mockup, TOC, local emulation, troubleshooting, and MIT license link.

---

## 4. Verification Plan

1. **Markdown Formatting Verification**:
   - Verify valid syntax, proper heading nesting, and functioning internal anchor links.
2. **Build & Test Verification**:
   - Run `make client` to verify native build.
   - Run `make test` to ensure all 33 unit tests pass.
   - Run `make test-asan` to verify memory sanitizers remain clean.
3. **Local Emulation Verification**:
   - Run `./bin/kindle-telegram` for 1 second in host fallback mode and check that `/tmp/kindle_fb.ppm` is generated.
4. **Git Status & Working Tree**:
   - Verify `git status` shows `LICENSE` untracked/added and `README.md` modified.
