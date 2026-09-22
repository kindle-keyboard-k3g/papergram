# Hardware Abstraction Layer (HAL)

The Hardware Abstraction Layer (`src/hal/`) isolates all Linux-specific kernel interfaces, memory-mapped I/O, evdev input streams, and network transport mechanisms. This enables running and testing Papergram both natively on target Kindle Keyboard hardware and in development mode on x86_64 host machines.

---

## 1. Framebuffer & Display

### `IFrameBuffer` (`src/hal/frame_buffer.h`)
Abstract interface for accessing the video memory buffer:
- `uint8_t* getBuffer()`: Returns raw pointer to the linear 600x800 8bpp memory buffer (480,000 bytes).
- `void flush()`: Commits modified memory pages to the video hardware.
- `int getWidth() const`, `int getHeight() const`: Returns display dimensions (600, 800).

### `FrameBufferLinuxFb0` (`src/hal/frame_buffer_fb0.h`)
Production implementation for Kindle Linux:
- Opens `/dev/fb0` with `O_RDWR`.
- Queries screen dimensions and bit depth using `FBIOGET_VSCREENINFO` and `FBIOGET_FSCREENINFO`.
- Maps the framebuffer into process virtual address space via `mmap()`.
- Validates double-buffering page offsets if enabled by the Kindle display driver.

### `IEinkController` (`src/hal/eink_controller.h`)
Abstract interface for controlling the E-Ink physical update engine:
- `void updateArea(int x, int y, int width, int height, EinkUpdateMode mode)`: Dispatches waveform refresh for the specified bounding box.
- `void waitComplete()`: Blocks until the current hardware update pipeline has finished rendering.

### `EinkControllerMxc` (`src/hal/eink_controller_mxc.h`)
Production driver for the Freescale / NXP i.MX35 Electronic Paper Display Controller (EPDC):
- Interacts with `/dev/fb0` using Kindle-specific ioctl `FBIO_EINK_UPDATE_DISPLAY_AREA` (`0x46dd`).
- Supported Waveform Modes:
  - `fx_update_partial` (DU mode): Fast 1-bit or 2-bit direct update waveform (~100ms) with minimal flash.
  - `fx_update_full` (GC16 mode): 16-level grayscale global flash waveform (~450ms) to reset particle polarization and eliminate ghosting.

---

## 2. Input Devices

### `IInputDevice` (`src/hal/input_device.h`)
Abstract interface for input ingestion:
- Defines standardized `Key` codes:
  - Navigation: `KEY_UP`, `KEY_DOWN`, `KEY_LEFT`, `KEY_RIGHT`, `KEY_ENTER`, `KEY_BACK`.
  - Side Rockers: `KEY_PAGE_UP`, `KEY_PAGE_DOWN`.
  - Top Level: `KEY_MENU`, `KEY_HOME`, `KEY_POWER`, `KEY_ALT`.
  - Alphanumeric: ASCII codes for physical keyboard typing.
- `bool pollEvent(InputEvent& event, int timeoutMs)`: Reads the next input event with a timeout.

### `InputDeviceEvdev` (`src/hal/input_device_evdev.h`)
Production driver multiplexing Linux evdev streams via `epoll()`:
- Monitors:
  - `/dev/input/event0`: Physical QWERTY keyboard.
  - `/dev/input/event1`: 5-way D-Pad navigation controller.
  - `/dev/input/event2`: Left and right page-turn rocker switches.
- Translates hardware scancodes to Papergram `Key` constants.
- Handles Shift and Alt key modifiers for secondary punctuation symbols (`+`, `@`, `!`, `?`, etc.).

---

## 3. Host Emulation & Fallback Devices

### `FallbackDevices` (`src/hal/fallback_devices.h`)
Enables full compilation, debugging, and automated headless testing on development workstations without `/dev/fb0`:
- **`MemoryFrameBuffer`**: Allocates an in-memory 480 KB byte buffer. Automatically dumps the screen contents to `/tmp/kindle_fb.ppm` on `flush()`.
- **`DummyEinkController`**: Accepts refresh commands without hardware ioctls, logging update areas when debug logging is active.
- **`StdinInputDevice`**: Reads keystrokes from standard input, translating terminal escape sequences and mapping characters (e.g. `~` to `KEY_POWER`, `m` to `KEY_MENU`).

---

## 4. Network Transport & Async Architecture

### `INetworkTransport` (`src/hal/network_transport.h`)
Interface for raw network connectivity:
- `bool send(const std::vector<uint8_t>& request, std::vector<uint8_t>& response)`: Executes MTProto transport transactions.
- `bool isConnected() const`: Queries transport connectivity status.

### `HttpTransport` (`src/hal/http_transport.h`)
Raw POSIX socket HTTP client implementing Telegram MTProto HTTP transport:
- Direct Wi-Fi connection to Telegram DC IP/port (`149.154.167.50:443`).
- Kindle 3G cellular proxy tunneling via Amazon endpoints (`fints.amazon.com:443`).
- Implements HTTP POST packaging with `Content-Type: application/octet-stream`.

### `AsyncWorker` (`src/hal/async_worker.h`)
Dedicated background worker thread for non-blocking operations:
- Offloads blocking operations (cryptographic handshake, network RPCs, file I/O).
- `postTask(AsyncTask task, UiCallback onComplete)`: Queues work for execution.
- `drainUiCallbacks()`: Executed by the main UI thread at each event loop tick to invoke completion handlers safely without multithreaded UI locks.
