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
    mapped_memory_[indexFor(coordinate)] = color.value();
}

GrayscaleColor FrameBufferLinuxFb0::getPixel(
    const ScreenCoordinate& coordinate) const {
    return GrayscaleColor(mapped_memory_[indexFor(coordinate)]);
}

void FrameBufferLinuxFb0::clear(const GrayscaleColor& color) {
    std::fill(mapped_memory_, mapped_memory_ + BUFFER_SIZE, color.value());
}

void FrameBufferLinuxFb0::flush() {
    if (::msync(mapped_memory_, BUFFER_SIZE, MS_SYNC) != 0) {
        throw std::system_error(errno, std::generic_category(),
                                "Unable to flush Linux framebuffer");
    }
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
