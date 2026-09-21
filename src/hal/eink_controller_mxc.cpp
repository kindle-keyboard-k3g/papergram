#include "eink_controller_mxc.h"

#include <cerrno>
#include <fcntl.h>
#include <stdexcept>
#include <system_error>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

struct EinkDisplayUpdate {
    int left;
    int top;
    int width;
    int height;
    int waveform;
};

}

EinkControllerMxc::EinkControllerMxc(const std::string& devicePath)
    : file_descriptor_(::open(devicePath.c_str(), O_RDWR)),
      owns_file_descriptor_(true) {
    if (file_descriptor_ < 0) {
        throw std::system_error(errno, std::generic_category(),
                                "Unable to open e-ink framebuffer");
    }
}

EinkControllerMxc::EinkControllerMxc(int framebufferFileDescriptor)
    : file_descriptor_(framebufferFileDescriptor),
      owns_file_descriptor_(false) {
    if (file_descriptor_ < 0) {
        throw std::invalid_argument("E-ink framebuffer descriptor is invalid");
    }
}

EinkControllerMxc::~EinkControllerMxc() {
    if (owns_file_descriptor_ && file_descriptor_ >= 0) {
        ::close(file_descriptor_);
    }
}

void EinkControllerMxc::updateArea(const BoundingBox& area,
                                   bool isFullRefresh) {
    const int waveform = isFullRefresh ? fx_update_full : fx_update_partial;
    issueUpdate(area, waveform);
}

void EinkControllerMxc::fullRefresh() {
    issueUpdate(BoundingBox(0, 0, 599, 799), fx_update_full);
}

int EinkControllerMxc::fileDescriptor() const {
    return file_descriptor_;
}

void EinkControllerMxc::issueUpdate(const BoundingBox& area, int waveform) const {
    EinkDisplayUpdate update{area.left(), area.top(), area.width(),
                             area.height(), waveform};
    if (::ioctl(file_descriptor_, FBIO_EINK_UPDATE_DISPLAY_AREA, &update) < 0) {
        throw std::system_error(errno, std::generic_category(),
                                "Unable to update e-ink display area");
    }
}
