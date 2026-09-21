CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2 -Isrc
DEBUG_FLAGS ?= -g -fsanitize=address,undefined

# Kindle ARM32 Toolchain Configuration (Kindle Keyboard / K3 i.MX35 ARMv6)
KINDLE_CXX ?= arm-linux-gnueabi-g++
KINDLE_TCC ?= kindle-tiny-c-compiler
KINDLE_FLAGS ?= -std=c++17 -O2 -march=armv6j -mtune=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp -Isrc

BIN_DIR := bin
SRC_DIR := src
TEST_DIR := tests

CORE_SRCS := $(wildcard $(SRC_DIR)/domain/*.cpp) \
             $(wildcard $(SRC_DIR)/graphics/*.cpp) \
             $(wildcard $(SRC_DIR)/hal/*.cpp) \
             $(wildcard $(SRC_DIR)/mtproto/*.cpp) \
             $(wildcard $(SRC_DIR)/ui/*.cpp)

APP_SRCS := $(CORE_SRCS) $(SRC_DIR)/main.cpp

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_CORE_SRCS := $(filter-out $(SRC_DIR)/main.cpp, $(APP_SRCS))

TARGET_CLIENT := $(BIN_DIR)/kindle-telegram
TARGET_TEST := $(BIN_DIR)/test_runner
TARGET_KINDLE := $(BIN_DIR)/kindle-telegram-arm32

.PHONY: all client test test-asan kindle clean help

all: client test

client: $(TARGET_CLIENT)

$(TARGET_CLIENT): $(APP_SRCS) | $(BIN_DIR)
	@echo "==> Building native client: $@"
	$(CXX) $(CXXFLAGS) $(APP_SRCS) -o $@

test: $(TARGET_TEST)
	@echo "==> Running full test suite..."
	@./$(TARGET_TEST)

$(TARGET_TEST): $(TEST_SRCS) $(TEST_CORE_SRCS) | $(BIN_DIR)
	@echo "==> Compiling test runner: $@"
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) $(TEST_CORE_SRCS) -o $@

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

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) *.ppm /tmp/kindle_*

help:
	@echo "Kindle Keyboard Telegram Client Build System"
	@echo "-------------------------------------------"
	@echo "make client    - Build native/host executable (bin/kindle-telegram)"
	@echo "make test      - Build and run zero-dependency test suite"
	@echo "make test-asan - Build and run tests with ASan + UBSan"
	@echo "make kindle    - Cross-compile ARM32 binary for Kindle Keyboard (K3)"
	@echo "make clean     - Clean build artifacts"
