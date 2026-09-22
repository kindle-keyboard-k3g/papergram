# Papergram Documentation Index 📚

Welcome to the internal engineering, architectural, and module documentation for **Papergram** — a standalone, serverless C++17 Telegram client engineered for the Amazon Kindle Keyboard (Kindle 3).

---

## Architecture & System Design

- [**Architecture Overview**](architecture/overview.md)  
  High-level architectural pipeline, component layering, embedded constraints, concurrency model, and event loop design.
- [**UI/UX Design System (Source of Truth)**](sot/ui_ux_design.md)  
  Complete specification for the **Modern Game Boy Aesthetics** on reflective E-Ink Pearl displays: 4-tone grayscale palette, double-bordered dialogue cards, D-pad ergonomics, and sunlight readability.

---

## Subsystem & Module Specifications

- [**Domain Layer Specification**](modules/domain.md)  
  Value objects (`PhoneNumber`, `AuthCode`, `BoundingBox`, `GrayscaleColor`), domain entities (`Chat`, `Message`), and first-class collections (`ChatList`, `MessageHistory`) compliant with Object Calisthenics.
- [**Graphics Subsystem & E-Ink Engine**](graphics/engine.md)  
  600x800 8bpp double-buffered `Canvas`, embedded monospace `BitmapFont`, `BufferDiffTracker`, `DarkToWhiteCleaner` anti-ghosting filter, and the 4x4 checkerboard `IdleRefreshScheduler`.
- [**Hardware Abstraction Layer (HAL)**](hardware/hal.md)  
  Linux framebuffer `/dev/fb0` memory mapping, Freescale i.MX35 EPDC ioctl driver (`0x46dd`), evdev input multiplexing, host fallback devices, HTTP transport, and asynchronous worker thread.
- [**MTProto 2.0 Client & Cryptography**](mtproto/client.md)  
  Zero-dependency cryptographic suite (AES-256-IGE, SHA-1, SHA-256, BigInteger modular exponentiation), Pollard's rho PQ factorization, Diffie-Hellman handshake, Type Language binary codec, and atomic session storage.
- [**UI Screens, Menus & Dialogs**](ui/screens_and_menus.md)  
  `ScreenNavigator` state machine, `LoginScreen` 3-step auth, `ChatListScreen`, `ConversationScreen`, `ScreensaverScreen`, physical `KindleMenu` overlay, and the unified `PopupManager`.

---

## Development & Operations

- [**Development, Build & Testing Guide**](development/build_and_test.md)  
  Host setup, compiler requirements, debug targets, AddressSanitizer tests, host PPM/stdin simulation, cross-compilation for ARM32, and physical device deployment via `scripts/launch_kindle.sh`.
- [**Engineering Implementation Plans**](plans/)  
  Historical and active implementation plans for asynchronous workers, UI flows, and menu overlays.
- [**Architectural Decision Records (ADR)**](adr/)  
  Decisions regarding architecture, protocol choices, and hardware interfaces.

---

## Project Guidelines

- [**Root User Guide & Quickstart**](../README.md)
- [**Coding Standards, Invariants & Guidelines**](../CLAUDE.md)
