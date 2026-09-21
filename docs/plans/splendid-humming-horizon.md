# Debug Build Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement full debug build support for Papergram (host debug binary, AddressSanitizer client, Kindle ARM32 debug target, `DEBUG=1` flag support, and zero-overhead conditional debug logging).

**Architecture:** Extend the `Makefile` with dedicated debug build targets (`make debug`, `make client-debug`, `make client-asan`, `make kindle-debug`, `make test-debug`) and dynamic `DEBUG=1` flag overrides. Introduce a zero-overhead compile-time debug logger in `src/util/debug_log.h` that emits structured subsystem traces (`[DEBUG][<Tag>] <message>`) when `-DDEBUG` is defined, and compiles to no-op code with 0 runtime overhead in release mode. Integrate debug traces at critical hardware and network boundaries (e-ink refresh strategy, input devices, and MTProto handshake).

**Tech Stack:** C++17, GNU Make, AddressSanitizer/UBSan, GCC/Clang, Linux evdev/fb0.

**Spec:** `docs/plans/splendid-humming-horizon.md`

## Global Constraints

- Strict Object Calisthenics: max 1 indentation level per method, 0 `else` keywords, classes ≤ 100 lines, methods ≤ 15 lines, ≤ 2 instance variables per class.
- Zero runtime overhead in release builds: release binaries must not include debug strings or logging code (target binary size < 350 KB).
- Zero third-party dependencies: standard library only (`<iostream>`, `<string>`, `<sstream>`).
- Comprehensive test coverage: all existing 34 tests must continue to pass, with new tests added for debug logging.

---

### Task 1: Zero-Overhead Debug Logging Utility

**Files:**
- Create: `src/util/debug_log.h`
- Test: `tests/test_debug_log.cpp`

**Interfaces:**
- Produces: `DebugLog::format(const std::string& tag, const std::string& message)` returning `std::string` formatted as `"[DEBUG][" + tag + "] " + message`
- Macro: `DEBUG_LOG(tag, message)` which streams to `std::clog` when `-DDEBUG` is active, and expands to `do {} while (0)` when `-DDEBUG` is absent.

- [x] **Step 1: Write the failing unit test for DebugLog**
Create `tests/test_debug_log.cpp` testing `DebugLog::format` and verifying output string structure.

- [x] **Step 2: Run test to verify it fails**
Run: `make test`
Expected: FAIL with compilation error (missing `src/util/debug_log.h`).

- [x] **Step 3: Implement `src/util/debug_log.h`**
Implement the header-only utility adhering to Object Calisthenics (0 `else`, single indent, small static formatter, conditional macro).

- [x] **Step 4: Run test to verify it passes**
Run: `make test`
Expected: PASS (all tests including `test_debug_log` pass).

- [x] **Step 5: Commit**
```bash
git add src/util/debug_log.h tests/test_debug_log.cpp
git commit -m "feat(debug): add zero-overhead debug logging utility and unit tests"
```

---

### Task 2: Integrate Debug Traces at Key Subsystem Boundaries

**Files:**
- Modify: `src/graphics/refresh_strategy.cpp`
- Modify: `src/hal/fallback_devices.cpp`
- Modify: `src/mtproto/handshake.cpp`

**Interfaces:**
- Consumes: `DEBUG_LOG(tag, message)` from `src/util/debug_log.h`

- [x] **Step 1: Add e-ink refresh logging**
In `src/graphics/refresh_strategy.cpp`, emit `DEBUG_LOG("Eink", ...)` on `TypingRefresh` (logging DU partial bounding box) and `FullRefresh` (logging GC16 full flash reason).

- [x] **Step 2: Add host input event logging**
In `src/hal/fallback_devices.cpp`, emit `DEBUG_LOG("Input", ...)` when mapping characters and escape sequences to `KeyCode`.

- [x] **Step 3: Add MTProto handshake logging**
In `src/mtproto/handshake.cpp`, emit `DEBUG_LOG("MTProto", ...)` during DH key derivation stages.

- [x] **Step 4: Verify test suite remains green**
Run: `make test && make test-asan`
Expected: All 35 tests pass cleanly.

- [x] **Step 5: Commit**
```bash
git add src/graphics/refresh_strategy.cpp src/hal/fallback_devices.cpp src/mtproto/handshake.cpp
git commit -m "feat(debug): add subsystem debug traces to e-ink, input, and mtproto"
```

---

### Task 3: Makefile Debug Targets & `DEBUG=1` Flag Configuration

**Files:**
- Modify: `Makefile`

**Interfaces:**
- Produces targets:
  * `make debug` / `make client-debug` -> `bin/kindle-telegram-debug` (`-g3 -O0 -DDEBUG -Isrc`)
  * `make client-asan` -> `bin/kindle-telegram-asan` (`-g -O1 -fsanitize=address,undefined -DDEBUG -Isrc`)
  * `make kindle-debug` -> `bin/kindle-telegram-arm32-debug` (`-g -O0 -DDEBUG` for Kindle gdbserver)
  * `make test-debug` -> `bin/test_runner_debug` (`-g3 -O0 -DDEBUG`)
  * Supports `DEBUG=1` parameter to dynamically switch base `CXXFLAGS` from `-O2` to `-g3 -O0 -DDEBUG`.
  * Updates `make help` with documentation for all debug commands.
  * Updates `make clean` to remove all debug binaries.

- [x] **Step 1: Update `Makefile` with debug flags and targets**
Add `DEBUG_CXXFLAGS`, `TARGET_CLIENT_DEBUG`, `TARGET_CLIENT_ASAN`, `TARGET_KINDLE_DEBUG`, `TARGET_TEST_DEBUG`, and the conditional `DEBUG=1` toggle.

- [x] **Step 2: Verify `make debug` builds correctly**
Run: `make debug`
Expected: Produces `bin/kindle-telegram-debug` with `-g3 -O0 -DDEBUG`. Verify symbols with `file bin/kindle-telegram-debug`.

- [x] **Step 3: Verify `make client-asan` builds correctly**
Run: `make client-asan`
Expected: Produces `bin/kindle-telegram-asan`. Verify with `timeout 1s ./bin/kindle-telegram-asan < /dev/null`.

- [x] **Step 4: Verify `make client DEBUG=1` builds with debug flags**
Run: `make clean && make client DEBUG=1`
Expected: Compiles with `-g3 -O0 -DDEBUG`.

- [x] **Step 5: Verify `make test` and `make test-debug`**
Run: `make test-debug`
Expected: Builds `bin/test_runner_debug` and passes all tests.

- [x] **Step 6: Commit**
```bash
git add Makefile
git commit -m "feat(build): add debug targets, client-asan, and DEBUG=1 support to Makefile"
```

---

### Task 4: Documentation & Developer Guidance Updates

**Files:**
- Modify: `CLAUDE.md`
- Modify: `README.md`

- [x] **Step 1: Update `CLAUDE.md`**
Document `make debug`, `make client-debug`, `make client-asan`, `make test-debug`, and `DEBUG=1` in the Build & Test Commands section.

- [x] **Step 2: Update `README.md`**
Add debug build instructions under the Quickstart & Local Host Emulation section, explaining GDB debugging and AddressSanitizer testing on host.

- [x] **Step 3: Verify documentation links and accuracy**
Check anchor links and ensure syntax formatting is consistent.

- [x] **Step 4: Commit**
```bash
git add CLAUDE.md README.md
git commit -m "docs: document debug builds, sanitizers, and GDB usage in CLAUDE.md and README.md"
```

---

## Verification Plan

1. **Host Debug Binary**:
   - Run `make clean && make debug` -> verify `bin/kindle-telegram-debug` exists, has debug symbols (`file bin/kindle-telegram-debug` shows `not stripped`), and runs with `< /dev/null`.
2. **Client ASan Binary**:
   - Run `make client-asan` -> verify `bin/kindle-telegram-asan` exists and executes cleanly without sanitizer warnings.
3. **Debug Flag Toggle (`DEBUG=1`)**:
   - Run `make client DEBUG=1` and `make test DEBUG=1`.
4. **Kindle Debug Target**:
   - Run `make kindle-debug` (or verify target syntax if cross-compiler is not on host).
5. **Unit Tests**:
   - Run `make test` and `make test-asan` -> verify all 35 tests pass cleanly.
6. **Clean Target**:
   - Run `make clean` -> verify all `bin/` artifacts are cleaned up.
