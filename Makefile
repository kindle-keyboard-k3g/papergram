CXX ?= g++

DEBUG_CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g3 -O0 -DDEBUG -Isrc
ASAN_CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g -O1 -fsanitize=address,undefined -DDEBUG -Isrc
DEBUG_FLAGS ?= -g -fsanitize=address,undefined

ifeq ($(DEBUG),1)
    CXXFLAGS := $(DEBUG_CXXFLAGS)
else
    CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2 -Isrc
endif

# Kindle ARM32 Toolchain Configuration (Kindle Keyboard / K3 i.MX35 ARMv6)
KINDLE_CXX ?= arm-linux-gnueabi-g++
KINDLE_TCC ?= kindle-tiny-c-compiler
KINDLE_FLAGS ?= -std=c++17 -O2 -march=armv6j -mtune=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp -Isrc
KINDLE_DEBUG_FLAGS ?= -std=c++17 -g -O0 -DDEBUG -march=armv6j -mtune=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp -Isrc

BIN_DIR := bin
SRC_DIR := src
TEST_DIR := tests

CORE_SRCS := $(wildcard $(SRC_DIR)/domain/*.cpp) \
             $(wildcard $(SRC_DIR)/graphics/*.cpp) \
             $(wildcard $(SRC_DIR)/hal/*.cpp) \
             $(wildcard $(SRC_DIR)/mtproto/*.cpp) \
             $(wildcard $(SRC_DIR)/ui/*.cpp) \
             $(wildcard $(SRC_DIR)/util/*.cpp)

APP_SRCS := $(CORE_SRCS) $(SRC_DIR)/main.cpp

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_CORE_SRCS := $(filter-out $(SRC_DIR)/main.cpp, $(APP_SRCS))

TARGET_CLIENT := $(BIN_DIR)/kindle-telegram
TARGET_CLIENT_DEBUG := $(BIN_DIR)/kindle-telegram-debug
TARGET_CLIENT_ASAN := $(BIN_DIR)/kindle-telegram-asan
TARGET_TEST := $(BIN_DIR)/test_runner
TARGET_TEST_DEBUG := $(BIN_DIR)/test_runner_debug
TARGET_KINDLE := $(BIN_DIR)/kindle-telegram-arm32
TARGET_KINDLE_DEBUG := $(BIN_DIR)/kindle-telegram-arm32-debug

.PHONY: all client debug client-debug client-asan test test-debug test-asan kindle kindle-debug clean help

all: client test

client: $(TARGET_CLIENT)

debug: client-debug

client-debug: $(TARGET_CLIENT_DEBUG)

client-asan: $(TARGET_CLIENT_ASAN)

$(TARGET_CLIENT): $(APP_SRCS) | $(BIN_DIR)
	@echo "==> Building native client: $@"
	$(CXX) $(CXXFLAGS) $(APP_SRCS) -o $@

$(TARGET_CLIENT_DEBUG): $(APP_SRCS) | $(BIN_DIR)
	@echo "==> Building native debug client: $@"
	$(CXX) $(DEBUG_CXXFLAGS) $(APP_SRCS) -o $@

$(TARGET_CLIENT_ASAN): $(APP_SRCS) | $(BIN_DIR)
	@echo "==> Building native AddressSanitizer client: $@"
	$(CXX) $(ASAN_CXXFLAGS) $(APP_SRCS) -o $@

test: $(TARGET_TEST)
	@echo "==> Running full test suite..."
	@./$(TARGET_TEST)

test-debug: $(TARGET_TEST_DEBUG)
	@echo "==> Running debug test suite..."
	@./$(TARGET_TEST_DEBUG)

$(TARGET_TEST): $(TEST_SRCS) $(TEST_CORE_SRCS) | $(BIN_DIR)
	@echo "==> Compiling test runner: $@"
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) $(TEST_CORE_SRCS) -o $@

$(TARGET_TEST_DEBUG): $(TEST_SRCS) $(TEST_CORE_SRCS) | $(BIN_DIR)
	@echo "==> Compiling debug test runner: $@"
	$(CXX) $(DEBUG_CXXFLAGS) $(TEST_SRCS) $(TEST_CORE_SRCS) -o $@

test-asan: | $(BIN_DIR)
	@echo "==> Compiling test suite with AddressSanitizer & UndefinedBehaviorSanitizer..."
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(TEST_SRCS) $(TEST_CORE_SRCS) -o $(BIN_DIR)/test_runner_asan
	@echo "==> Running Sanitizer test suite..."
	@./$(BIN_DIR)/test_runner_asan
	@rm -f $(BIN_DIR)/test_runner_asan

kindle: $(APP_SRCS) | $(BIN_DIR)
	@echo "==> Cross-compiling for Kindle Keyboard (ARMv6 softfp)..."
	@if command -v "$(KINDLE_CXX)" >/dev/null 2>&1; then \
		$(KINDLE_CXX) $(KINDLE_FLAGS) -static-libstdc++ $(APP_SRCS) -o $(TARGET_KINDLE); \
	elif command -v "$(KINDLE_TCC)" >/dev/null 2>&1; then \
		echo "[INFO] Using Kindle tiny C/C++ compiler: $(KINDLE_TCC)"; \
		$(KINDLE_TCC) $(KINDLE_FLAGS) $(APP_SRCS) -o $(TARGET_KINDLE); \
	else \
		echo "[ERROR] No Kindle ARM32 compiler found." >&2; \
		echo "Install $(KINDLE_CXX) or $(KINDLE_TCC), or invoke make with an explicit toolchain path." >&2; \
		exit 1; \
	fi
	@test -x $(TARGET_KINDLE)
	@echo "[SUCCESS] Built Kindle ARM32 binary at $(TARGET_KINDLE)"
	@ls -lh $(TARGET_KINDLE)

kindle-debug: $(APP_SRCS) | $(BIN_DIR)
	@echo "==> Cross-compiling debug binary for Kindle Keyboard (ARMv6 softfp)..."
	@if command -v "$(KINDLE_CXX)" >/dev/null 2>&1; then \
		$(KINDLE_CXX) $(KINDLE_DEBUG_FLAGS) -static-libstdc++ $(APP_SRCS) -o $(TARGET_KINDLE_DEBUG); \
	elif command -v "$(KINDLE_TCC)" >/dev/null 2>&1; then \
		echo "[INFO] Using Kindle tiny C/C++ compiler: $(KINDLE_TCC)"; \
		$(KINDLE_TCC) $(KINDLE_DEBUG_FLAGS) $(APP_SRCS) -o $(TARGET_KINDLE_DEBUG); \
	else \
		echo "[ERROR] No Kindle ARM32 compiler found." >&2; \
		echo "Install $(KINDLE_CXX) or $(KINDLE_TCC), or invoke make with an explicit toolchain path." >&2; \
		exit 1; \
	fi
	@test -x $(TARGET_KINDLE_DEBUG)
	@echo "[SUCCESS] Built Kindle ARM32 debug binary at $(TARGET_KINDLE_DEBUG)"
	@ls -lh $(TARGET_KINDLE_DEBUG)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) *.ppm /tmp/kindle_*

help:
	@echo "Kindle Keyboard Telegram Client Build System"
	@echo "-------------------------------------------"
	@echo "make client       - Build native/host executable (bin/kindle-telegram)"
	@echo "make debug        - Build native debug executable (bin/kindle-telegram-debug)"
	@echo "make client-debug - Alias for make debug"
	@echo "make client-asan  - Build native ASan+UBSan executable (bin/kindle-telegram-asan)"
	@echo "make test         - Build and run zero-dependency test suite"
	@echo "make test-debug   - Build and run debug test suite with traces"
	@echo "make test-asan    - Build and run tests with ASan + UBSan"
	@echo "make kindle       - Cross-compile ARM32 binary for Kindle Keyboard (K3)"
	@echo "make kindle-debug - Cross-compile ARM32 debug binary with symbols for Kindle"
	@echo "make clean        - Clean build artifacts"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1         - Compile with -g3 -O0 -DDEBUG enabled"
