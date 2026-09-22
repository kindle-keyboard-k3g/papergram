# Graphics Subsystem & E-Ink Engine

The graphics subsystem (`src/graphics/`) drives the 600x800 8bpp display of the Amazon Kindle Keyboard (Kindle 3). It translates high-level UI component layout into pixel buffers and coordinates optimal hardware waveform updates to eliminate e-ink ghosting while maintaining snappy keystroke response.

---

## 1. Core Components

### `Canvas` (`src/graphics/canvas.h`)
Double-buffered drawing surface with dimensions 600x800 at 8 bits per pixel (1 byte = 1 grayscale pixel: 0x00=black, 0xFF=white).
- **Buffer Layout**: Maintains a `backBuffer` (active drawing target) and a `frontBuffer` (snapshot of currently displayed frame on the hardware).
- **Drawing Primitives**:
  - `clear(GrayscaleColor color)`: Fills entire canvas.
  - `setPixel(int x, int y, GrayscaleColor color)` / `setPixel(ScreenCoordinate, GrayscaleColor)`: Writes a clipped single pixel.
  - `drawLine(int x1, int y1, int x2, int y2, GrayscaleColor color)`: Bresenham line rasterizer.
  - `drawRect(int left, int top, int right, int bottom, GrayscaleColor color)`: Draws 1px outline with inclusive bounds.
  - `fillRect(int left, int top, int right, int bottom, GrayscaleColor color)`: Solid rectangular fill with inclusive bounds.
  - `invertRect(int left, int top, int right, int bottom)`: Bitwise pixel inversion for cursor and selection highlighting.
  - `blitText(int x, int y, const std::string& text, GrayscaleColor color)`: Monospace 8x16 text blitting.
  - `saveToPpm(const std::string& filepath)`: Exports backBuffer to PPM image format (used by host fallback and test assertions).
  - `frontBuffer()` / `backBuffer()`: Direct pointer access to raw 480 KB linear pixel arrays.
- **Memory Footprint**: Exactly 480 KB per buffer (960 KB total RAM).

### `BitmapFont` (`src/graphics/bitmap_font.h`)
Embedded monospace bitmap font designed for high contrast and zero runtime memory allocation:
- **Glyph Size**: 8 pixels wide by 16 pixels tall.
- **Coverage**: Full printable ASCII table (`0x20` space to `0x7E` tilde) plus common Latin-1 characters.
- **Internal Storage**: Encoded as raw bit-packed 1bpp glyph arrays compiled directly into read-only flash memory.
- **Helper Utilities**: Text length measurement, word wrapping at arbitrary character boundaries (standard 58 characters for conversation bubbles).

---

## 2. E-Ink Ghosting Mitigation Pipeline

E-Ink Pearl displays suffer from residual image retention ("ghosting"), especially when rendering dark text or inverted selection bars that are subsequently overwritten with white pixels. Papergram implements a 4-tier mitigation strategy:

```
                          [User Input / Screen Redraw]
                                      |
                                      v
                                Canvas Draw
                                      |
                                      v
                             BufferDiffTracker
                        (Compares Front vs Back Buffer)
                                  /       \
                                 /         \
            General Changed Regions         Dark-to-White Transitions
                   |                                    |
                   v                                    v
          DirtyRegionList                      DarkToWhiteCleaner
                   |                         (Issues immediate DU pass)
                   v                                    |
         EinkRefreshStrategy <--------------------------+
      (Typing DU vs GC16 Flash)
                   |
                   v
             Hardware ioctl
                   |
       (During >5s Inactivity)
                   v
        IdleRefreshScheduler
    (16-tile Interleaved GC16)
```

### `DirtyTracker` (`src/graphics/dirty_tracker.h`)
Aggregates bounding boxes of updated regions to minimize the area flashed on the display:
- Merges adjacent or overlapping rectangles to reduce the total count of ioctl calls.
- Preserves disjoint regions to avoid needlessly refreshing untouched screen areas.

### `BufferDiffTracker` (`src/graphics/diff_tracker.h`)
Performs a byte-by-byte comparison between `Canvas::getFrontBuffer()` and `Canvas::getBackBuffer()`:
- Identifies the minimal enclosing bounding box of changed pixels.
- Identifies **dark-to-white transitions** (pixels moving from `BLACK` or `DARK_GRAY` to `WHITE`), which are the primary source of ghosting on Pearl displays.
- Caps excessive fragmentation: if the bounding box count exceeds a threshold, it coalesces them into a single full-screen bounding box to avoid ioctl queue starvation.

### `DarkToWhiteCleaner` (`src/graphics/dark_to_white_cleaner.h`)
Consumes dark-to-white regions reported by `BufferDiffTracker`:
- Issues an immediate secondary DU (Direct Update) cleaning pass across affected regions.
- Forces physical particles to settle into a clean white state before rendering final content, effectively erasing ghost artifacts.

### `IdleRefreshScheduler` (`src/graphics/idle_refresh_scheduler.h`)
Runs autonomous screen maintenance when the user is idle:
- **Activation Threshold**: 5 seconds of inactivity (`noteActivity()` called on any keystroke resets the timer).
- **Sweep Pattern**: Divides the 600x800 screen into a 4x4 grid of 16 tiles (each 150x200 pixels).
- **Interleaved Order**: Refreshes tiles in a pseudo-random or bit-reversed order spaced 400ms apart using full GC16 waveform.
- **Instant Preemption**: If a keypress occurs during an active sweep, the scheduler aborts immediately without blocking the UI thread.

### `EinkRefreshStrategy` (`src/graphics/refresh_strategy.h`)
Controls waveform selection according to user interaction modes:
- **`TypingRefresh`**: Uses fast DU partial updates for typing and cursor movement (<120ms latency).
- **`FullRefresh`**: Triggers a global GC16 flash:
  - Every 15 consecutive typing keystrokes.
  - On major screen transitions (e.g., entering a chat, opening settings).
  - On user command via `Alt+G` ("Ghostbuster").
