#include "frame_buffer_fb0.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <system_error>
#include <sys/mman.h>
#include <unistd.h>

FrameBufferLinuxFb0::FrameBufferLinuxFb0(const std::string& devicePath)
    : file_descriptor_(::open(devicePath.c_str(), O_RDWR)),
      mapped_memory_(nullptr) {
    if (file_descriptor_ < 0) {
        throw std::system_error(errno, std::generic_category(),
                                "Unable to open Linux framebuffer");
    }
    void* mapping = ::mmap(nullptr, BUFFER_SIZE, PROT_READ | PROT_WRITE,
                           MAP_SHARED, file_descriptor_, 0);
    if (mapping == MAP_FAILED) {
        const int error = errno;
        ::close(file_descriptor_);
        file_descriptor_ = -1;
        throw std::system_error(error, std::generic_category(),
                                "Unable to map Linux framebuffer");
    }
    mapped_memory_ = static_cast<unsigned char*>(mapping);
}

FrameBufferLinuxFb0::~FrameBufferLinuxFb0() {
    if (mapped_memory_ != nullptr) {
        ::munmap(mapped_memory_, BUFFER_SIZE);
    }
    if (file_descriptor_ >= 0) {
        ::close(file_descriptor_);
    }
}

std::size_t FrameBufferLinuxFb0::width() const {
    return WIDTH;
}

std::size_t FrameBufferLinuxFb0::height() const {
    return HEIGHT;
}

void FrameBufferLinuxFb0::setPixel(const ScreenCoordinate& coordinate,
                                   const GrayscaleColor& color) {
    const std::size_t byte_index = indexFor(coordinate) / 2U;
    const std::uint8_t nibble = static_cast<std::uint8_t>((255U - color.value()) >> 4U);
    setEvenPixel(byte_index, nibble, coordinate.x());
    setOddPixel(byte_index, nibble, coordinate.x());
}

void FrameBufferLinuxFb0::setEvenPixel(std::size_t byte_index, std::uint8_t nibble, int x) {
    if ((x & 1) != 0) return;
    mapped_memory_[byte_index] = (mapped_memory_[byte_index] & 0x0FU) | (nibble << 4U);
}

void FrameBufferLinuxFb0::setOddPixel(std::size_t byte_index, std::uint8_t nibble, int x) {
    if ((x & 1) == 0) return;
    mapped_memory_[byte_index] = (mapped_memory_[byte_index] & 0xF0U) | (nibble & 0x0FU);
}

GrayscaleColor FrameBufferLinuxFb0::getPixel(
    const ScreenCoordinate& coordinate) const {
    const std::size_t byte_index = indexFor(coordinate) / 2U;
    const std::uint8_t byte_val = mapped_memory_[byte_index];
    const std::uint8_t nibble = ((coordinate.x() & 1) == 0)
                                    ? ((byte_val >> 4U) & 0x0FU)
                                    : (byte_val & 0x0FU);
    const std::uint8_t color_val = static_cast<std::uint8_t>(255U - (nibble << 4U));
    return GrayscaleColor(color_val);
}

void FrameBufferLinuxFb0::clear(const GrayscaleColor& color) {
    const std::uint8_t nibble = static_cast<std::uint8_t>((255U - color.value()) >> 4U);
    const std::uint8_t byte_val = static_cast<std::uint8_t>((nibble << 4U) | nibble);
    std::fill(mapped_memory_, mapped_memory_ + BUFFER_SIZE, byte_val);
}

void FrameBufferLinuxFb0::copyFrom(const std::uint8_t* buffer, std::size_t size) {
    if (buffer == nullptr || mapped_memory_ == nullptr) return;
    pack8bppTo4bpp(buffer, std::min(size, WIDTH * HEIGHT));
}

void FrameBufferLinuxFb0::pack8bppTo4bpp(const std::uint8_t* buffer, std::size_t count) {
    const std::size_t pairs = count / 2U;
    for (std::size_t i = 0; i < pairs; ++i) {
        const std::uint8_t n0 = static_cast<std::uint8_t>((255U - buffer[i * 2U]) >> 4U);
        const std::uint8_t n1 = static_cast<std::uint8_t>((255U - buffer[i * 2U + 1U]) >> 4U);
        mapped_memory_[i] = static_cast<std::uint8_t>((n0 << 4U) | (n1 & 0x0FU));
    }
}

void FrameBufferLinuxFb0::flush() {
    // Framebuffer memory is mapped with MAP_SHARED directly to video RAM.
    // Display updates are signaled to the e-ink controller via ioctls.
}

int FrameBufferLinuxFb0::fileDescriptor() const {
    return file_descriptor_;
}

std::size_t FrameBufferLinuxFb0::indexFor(
    const ScreenCoordinate& coordinate) {
    if (coordinate.x() < 0 || coordinate.x() >= static_cast<int>(WIDTH) ||
        coordinate.y() < 0 || coordinate.y() >= static_cast<int>(HEIGHT)) {
        throw std::out_of_range("Framebuffer coordinate is outside the display");
    }
    return static_cast<std::size_t>(coordinate.y()) * WIDTH +
           static_cast<std::size_t>(coordinate.x());
}
