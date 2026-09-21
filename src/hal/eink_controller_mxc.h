#ifndef KINDLE_HAL_EINK_CONTROLLER_MXC_H
#define KINDLE_HAL_EINK_CONTROLLER_MXC_H

#include "eink_controller.h"

#include <string>

class EinkControllerMxc final : public IEinkController {
public:
    explicit EinkControllerMxc(const std::string& devicePath = "/dev/fb0");
    explicit EinkControllerMxc(int framebufferFileDescriptor);
    ~EinkControllerMxc() override;

    EinkControllerMxc(const EinkControllerMxc&) = delete;
    EinkControllerMxc& operator=(const EinkControllerMxc&) = delete;

    void updateArea(const BoundingBox& area, bool isFullRefresh) override;
    void fullRefresh() override;
    int fileDescriptor() const;

private:
    int file_descriptor_;
    bool owns_file_descriptor_;

    void issueUpdate(const BoundingBox& area, int waveform) const;
};

#endif
