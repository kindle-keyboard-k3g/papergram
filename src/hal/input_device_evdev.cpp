#include "input_device_evdev.h"
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <unistd.h>

namespace {
constexpr std::uint16_t LINUX_KEY_0 = KEY_0;
constexpr std::uint16_t LINUX_KEY_1 = KEY_1;
constexpr std::uint16_t LINUX_KEY_9 = KEY_9;
constexpr std::uint16_t LINUX_KEY_ENTER = KEY_ENTER;
constexpr std::uint16_t LINUX_KEY_BACKSPACE = KEY_BACKSPACE;
constexpr std::uint16_t LINUX_KEY_SPACE = KEY_SPACE;
constexpr std::uint16_t LINUX_KEY_UP = KEY_UP;
constexpr std::uint16_t LINUX_KEY_DOWN = KEY_DOWN;
constexpr std::uint16_t LINUX_KEY_LEFT = KEY_LEFT;
constexpr std::uint16_t LINUX_KEY_RIGHT = KEY_RIGHT;
constexpr std::uint16_t LINUX_KEY_PAGEUP = KEY_PAGEUP;
constexpr std::uint16_t LINUX_KEY_PAGEDOWN = KEY_PAGEDOWN;
constexpr std::uint16_t LINUX_KEY_ESC = KEY_ESC;

constexpr std::uint16_t LINUX_KEY_A = KEY_A;
constexpr std::uint16_t LINUX_KEY_B = KEY_B;
constexpr std::uint16_t LINUX_KEY_C = KEY_C;
constexpr std::uint16_t LINUX_KEY_D = KEY_D;
constexpr std::uint16_t LINUX_KEY_E = KEY_E;
constexpr std::uint16_t LINUX_KEY_F = KEY_F;
constexpr std::uint16_t LINUX_KEY_G = KEY_G;
constexpr std::uint16_t LINUX_KEY_H = KEY_H;
constexpr std::uint16_t LINUX_KEY_I = KEY_I;
constexpr std::uint16_t LINUX_KEY_J = KEY_J;
constexpr std::uint16_t LINUX_KEY_K = KEY_K;
constexpr std::uint16_t LINUX_KEY_L = KEY_L;
constexpr std::uint16_t LINUX_KEY_M = KEY_M;
constexpr std::uint16_t LINUX_KEY_N = KEY_N;
constexpr std::uint16_t LINUX_KEY_O = KEY_O;
constexpr std::uint16_t LINUX_KEY_P = KEY_P;
constexpr std::uint16_t LINUX_KEY_Q = KEY_Q;
constexpr std::uint16_t LINUX_KEY_R = KEY_R;
constexpr std::uint16_t LINUX_KEY_S = KEY_S;
constexpr std::uint16_t LINUX_KEY_T = KEY_T;
constexpr std::uint16_t LINUX_KEY_U = KEY_U;
constexpr std::uint16_t LINUX_KEY_V = KEY_V;
constexpr std::uint16_t LINUX_KEY_W = KEY_W;
constexpr std::uint16_t LINUX_KEY_X = KEY_X;
constexpr std::uint16_t LINUX_KEY_Y = KEY_Y;
constexpr std::uint16_t LINUX_KEY_Z = KEY_Z;
}

#undef KEY_0
#undef KEY_1
#undef KEY_9
#undef KEY_ENTER
#undef KEY_BACKSPACE
#undef KEY_SPACE
#undef KEY_UP
#undef KEY_DOWN
#undef KEY_LEFT
#undef KEY_RIGHT
#undef KEY_PAGEUP
#undef KEY_PAGEDOWN
#undef KEY_ESC
#undef KEY_BACK
#undef KEY_A
#undef KEY_B
#undef KEY_C
#undef KEY_D
#undef KEY_E
#undef KEY_F
#undef KEY_G
#undef KEY_H
#undef KEY_I
#undef KEY_J
#undef KEY_K
#undef KEY_L
#undef KEY_M
#undef KEY_N
#undef KEY_O
#undef KEY_P
#undef KEY_Q
#undef KEY_R
#undef KEY_S
#undef KEY_T
#undef KEY_U
#undef KEY_V
#undef KEY_W
#undef KEY_X
#undef KEY_Y
#undef KEY_Z

namespace {
bool translateLetter(std::uint16_t code, KeyCode& out) {
    static const std::uint16_t letter_codes[26] = {
        LINUX_KEY_A, LINUX_KEY_B, LINUX_KEY_C, LINUX_KEY_D, LINUX_KEY_E, LINUX_KEY_F,
        LINUX_KEY_G, LINUX_KEY_H, LINUX_KEY_I, LINUX_KEY_J, LINUX_KEY_K, LINUX_KEY_L,
        LINUX_KEY_M, LINUX_KEY_N, LINUX_KEY_O, LINUX_KEY_P, LINUX_KEY_Q, LINUX_KEY_R,
        LINUX_KEY_S, LINUX_KEY_T, LINUX_KEY_U, LINUX_KEY_V, LINUX_KEY_W, LINUX_KEY_X,
        LINUX_KEY_Y, LINUX_KEY_Z
    };
    for (std::size_t i = 0; i < 26; ++i) {
        if (letter_codes[i] == code) {
            out = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + i);
            return true;
        }
    }
    return false;
}

bool translateNavigation(std::uint16_t code, KeyCode& out) {
    if (code == LINUX_KEY_UP) { out = KeyCode::KEY_UP; return true; }
    if (code == LINUX_KEY_DOWN) { out = KeyCode::KEY_DOWN; return true; }
    if (code == LINUX_KEY_LEFT) { out = KeyCode::KEY_LEFT; return true; }
    if (code == LINUX_KEY_RIGHT) { out = KeyCode::KEY_RIGHT; return true; }
    if (code == LINUX_KEY_PAGEUP || code == 193 || code == 109) {
        out = KeyCode::KEY_PAGEUP;
        return true;
    }
    if (code == LINUX_KEY_PAGEDOWN || code == 104 || code == 191) {
        out = KeyCode::KEY_PAGEDOWN;
        return true;
    }
    return false;
}

bool translateSpecial(std::uint16_t code, KeyCode& out) {
    if (code >= LINUX_KEY_1 && code <= LINUX_KEY_9) {
        out = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_1) + (code - LINUX_KEY_1));
        return true;
    }
    if (code == LINUX_KEY_0) { out = KeyCode::KEY_0; return true; }
    if (code == LINUX_KEY_ENTER || code == 194) { out = KeyCode::KEY_ENTER; return true; }
    if (code == LINUX_KEY_BACKSPACE) { out = KeyCode::KEY_BACKSPACE; return true; }
    if (code == LINUX_KEY_SPACE) { out = KeyCode::KEY_SPACE; return true; }
    if (code == LINUX_KEY_ESC || code == 158) { out = KeyCode::KEY_BACK; return true; }
    return translateNavigation(code, out);
}
}

InputDeviceEvdev::InputDeviceEvdev() {
    openDevices();
}

InputDeviceEvdev::~InputDeviceEvdev() {
    for (int fd : fds_) {
        if (fd >= 0) close(fd);
    }
}

void InputDeviceEvdev::openDevices() {
    const char* paths[] = {"/dev/input/event0", "/dev/input/event1", "/dev/input/event2"};
    for (const char* path : paths) {
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd >= 0) {
            fds_.push_back(fd);
            has_devices_ = true;
        }
    }
}

bool InputDeviceEvdev::pollEvent(InputEvent& out_event, int timeout_ms) {
    if (!has_devices_) return false;
    std::vector<struct pollfd> pfd(fds_.size());
    for (std::size_t i = 0; i < fds_.size(); ++i) {
        pfd[i].fd = fds_[i];
        pfd[i].events = POLLIN;
    }
    int ret = poll(pfd.data(), pfd.size(), timeout_ms);
    if (ret <= 0) return false;
    for (std::size_t i = 0; i < fds_.size(); ++i) {
        if (pfd[i].revents & POLLIN) {
            struct input_event ev;
            if (read(fds_[i], &ev, sizeof(ev)) == sizeof(ev)) {
                return translateCode(ev.type, ev.code, ev.value, out_event);
            }
        }
    }
    return false;
}

bool InputDeviceEvdev::translateCode(std::uint16_t type, std::uint16_t code, std::int32_t value, InputEvent& out) const {
    if (type != EV_KEY) return false;
    out.pressed = (value != 0);
    if (translateLetter(code, out.code)) return true;
    if (translateSpecial(code, out.code)) return true;
    return false;
}
