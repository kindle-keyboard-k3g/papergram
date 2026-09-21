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
    switch (code) {
        case LINUX_KEY_A: out = KeyCode::KEY_A; return true;
        case LINUX_KEY_B: out = KeyCode::KEY_B; return true;
        case LINUX_KEY_C: out = KeyCode::KEY_C; return true;
        case LINUX_KEY_D: out = KeyCode::KEY_D; return true;
        case LINUX_KEY_E: out = KeyCode::KEY_E; return true;
        case LINUX_KEY_F: out = KeyCode::KEY_F; return true;
        case LINUX_KEY_G: out = KeyCode::KEY_G; return true;
        case LINUX_KEY_H: out = KeyCode::KEY_H; return true;
        case LINUX_KEY_I: out = KeyCode::KEY_I; return true;
        case LINUX_KEY_J: out = KeyCode::KEY_J; return true;
        case LINUX_KEY_K: out = KeyCode::KEY_K; return true;
        case LINUX_KEY_L: out = KeyCode::KEY_L; return true;
        case LINUX_KEY_M: out = KeyCode::KEY_M; return true;
        case LINUX_KEY_N: out = KeyCode::KEY_N; return true;
        case LINUX_KEY_O: out = KeyCode::KEY_O; return true;
        case LINUX_KEY_P: out = KeyCode::KEY_P; return true;
        case LINUX_KEY_Q: out = KeyCode::KEY_Q; return true;
        case LINUX_KEY_R: out = KeyCode::KEY_R; return true;
        case LINUX_KEY_S: out = KeyCode::KEY_S; return true;
        case LINUX_KEY_T: out = KeyCode::KEY_T; return true;
        case LINUX_KEY_U: out = KeyCode::KEY_U; return true;
        case LINUX_KEY_V: out = KeyCode::KEY_V; return true;
        case LINUX_KEY_W: out = KeyCode::KEY_W; return true;
        case LINUX_KEY_X: out = KeyCode::KEY_X; return true;
        case LINUX_KEY_Y: out = KeyCode::KEY_Y; return true;
        case LINUX_KEY_Z: out = KeyCode::KEY_Z; return true;
        default: return false;
    }
}

bool translateSpecial(std::uint16_t code, KeyCode& out) {
    if (code >= LINUX_KEY_1 && code <= LINUX_KEY_9) {
        out = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_1) + (code - LINUX_KEY_1));
        return true;
    }
    if (code == LINUX_KEY_0) { out = KeyCode::KEY_0; return true; }
    if (code == LINUX_KEY_ENTER) { out = KeyCode::KEY_ENTER; return true; }
    if (code == LINUX_KEY_BACKSPACE) { out = KeyCode::KEY_BACKSPACE; return true; }
    if (code == LINUX_KEY_SPACE) { out = KeyCode::KEY_SPACE; return true; }
    if (code == LINUX_KEY_UP) { out = KeyCode::KEY_UP; return true; }
    if (code == LINUX_KEY_DOWN) { out = KeyCode::KEY_DOWN; return true; }
    if (code == LINUX_KEY_LEFT) { out = KeyCode::KEY_LEFT; return true; }
    if (code == LINUX_KEY_RIGHT) { out = KeyCode::KEY_RIGHT; return true; }
    if (code == LINUX_KEY_PAGEUP) { out = KeyCode::KEY_PAGEUP; return true; }
    if (code == LINUX_KEY_PAGEDOWN) { out = KeyCode::KEY_PAGEDOWN; return true; }
    if (code == LINUX_KEY_ESC || code == 158) { out = KeyCode::KEY_BACK; return true; }
    return false;
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
