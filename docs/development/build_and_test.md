# Development, Build & Testing Guide

This document covers toolchain setup, compilation targets, test execution, debugging on host environments, and cross-compiling for Amazon Kindle Keyboard (Kindle 3) hardware.

---

## 1. Prerequisites

### Native Host Development (Linux / WSL2)
- **Compiler**: Modern C++17 compiler (`g++` >= 9.0 or `clang++` >= 10.0).
- **Build System**: GNU Make.
- **Dependencies**: None. Papergram has zero third-party library dependencies.

### Target Hardware Cross-Compilation
- **Kindle 3 Toolchain**: `arm-linux-gnueabi-g++` (targeting ARMv6/ARM1136JF-S architecture with glibc 2.5/2.9 compatibility) or the repository's `kindle-tiny-c-compiler`.

---

## 2. Build Targets

The repository provides a modular `Makefile` with targets for development, debugging, sanitization, and production release:

| Command | Output Binary | Flags & Features | Use Case |
|---|---|---|---|
| `make client` | `bin/kindle-telegram` | `-std=c++17 -O2 -Wall -Wextra -pedantic` | Native host client binary |
| `make debug` / `make client-debug` | `bin/kindle-telegram-debug` | `-std=c++17 -g3 -O0 -DDEBUG` | Host debugging with symbols & logging |
| `make client-asan` | `bin/kindle-telegram-asan` | `-fsanitize=address,undefined -g -O1 -DDEBUG` | Memory safety & leak detection |
| `make kindle` | `bin/kindle-telegram-arm32` | Cross-compiled for ARM32 | Release binary for physical Kindle device |
| `make kindle-debug` | `bin/kindle-telegram-arm32-debug` | Cross-compiled ARM32 with `-g -DDEBUG` | On-device remote debugging with `gdbserver` |
| `make clean` | Removes `bin/` and `build/` | Clean build workspace | Pre-commit cleanup |

---

## 3. Running & Testing

### Running Full Test Suite
```bash
make test
```
Builds `bin/test_runner` and executes all unit tests covering cryptography, TL codec, graphics canvas, buffer diffing, e-ink refresh schedulers, UI screens, popups, and menus.

### Running Tests in Debug Mode
```bash
make test-debug
```
Enables verbose runtime debug logs (`DEBUG_LOG`) during test execution.

### Running Tests with AddressSanitizer & UBSan
```bash
make test-asan
```
Compiles and runs the full test suite under Clang/GCC AddressSanitizer and UndefinedBehaviorSanitizer to verify zero buffer overflows, memory leaks, or unaligned pointer access.

### Running a Single Test File
To run a specific test suite during development without running all 140+ tests:
```bash
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -Isrc tests/test_runner.cpp tests/test_<name>.cpp $(filter-out src/main.cpp, $(wildcard src/*/*.cpp)) -o bin/test_single && ./bin/test_single
```

---

## 4. Host Emulation & Development Workflow

When launched on a host development machine without access to `/dev/fb0`:
```bash
./bin/kindle-telegram
```
1. **Framebuffer Fallback**: Papergram automatically detects the missing framebuffer device and instantiates `hal::MemoryFrameBuffer`. Every screen flush outputs the current frame to `/tmp/kindle_fb.ppm`.
2. **Display Viewing**: You can inspect the screen output in real-time using `feh` or `imagemagick`:
   ```bash
   feh --reload 0.5 /tmp/kindle_fb.ppm
   ```
3. **Input Fallback**: Standard terminal input is captured via `hal::StdinInputDevice`. Special keys:
   - `~` = Power Switch (locks / unlocks device)
   - `m` = Menu Button (opens Kindle menu overlay)
   - Arrow keys = D-Pad navigation
   - Enter = Select / Confirm
   - Esc / Backspace = Back

---

## 5. Deployment to Physical Kindle Keyboard

Deploying and running on a jailbroken Kindle Keyboard:

1. **Mount Kindle via SSH**: Connect over USB network (`usbnet` interface on `192.168.15.244` or `192.168.2.2`).
2. **Stop Amazon Java Framework**:
   The stock Amazon e-reader framework (`cvm` / `awesome` window manager) consumes ~120 MB RAM and continuously polls `/dev/fb0`. It must be temporarily stopped before launching Papergram:
   ```bash
   /etc/init.d/framework stop
   ```
3. **Launch Binary**:
   ```bash
   /mnt/us/telegram/kindle-telegram-arm32
   ```
4. **Restore Framework on Exit**:
   ```bash
   /etc/init.d/framework start
   ```
   *Note: `scripts/launch_kindle.sh` automates steps 2-4 with signal trapping.*
