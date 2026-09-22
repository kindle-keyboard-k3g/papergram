#ifndef KINDLE_HAL_EINK_CONTROLLER_MXC_H
#define KINDLE_HAL_EINK_CONTROLLER_MXC_H

#include "eink_controller.h"

#include <string>

/**
 * @brief E-ink controller implementation for the Kindle MXC framebuffer.
 *
 * The controller sends refresh requests through the Kindle framebuffer ioctl.
 */
class EinkControllerMxc final : public IEinkController {
public:
    /**
     * @brief Opens a framebuffer device for e-ink refresh control.
     * @param devicePath Path to the framebuffer device.
     * @throws std::system_error If the device cannot be opened.
     */
    explicit EinkControllerMxc(const std::string& devicePath = "/dev/fb0");

    /**
     * @brief Uses an existing framebuffer file descriptor.
     * @param framebufferFileDescriptor Open framebuffer descriptor to use.
     * @throws std::invalid_argument If the descriptor is negative.
     */
    explicit EinkControllerMxc(int framebufferFileDescriptor);

    /** @brief Closes the framebuffer descriptor when this object owns it. */
    ~EinkControllerMxc() override;

    /** @brief Disables copying because the controller owns a device resource. */
    EinkControllerMxc(const EinkControllerMxc&) = delete;

    /** @brief Disables assignment because the controller owns a device resource. */
    EinkControllerMxc& operator=(const EinkControllerMxc&) = delete;

    /**
     * @brief Refreshes a display region using the selected waveform.
     * @param area Region of the display to refresh.
     * @param isFullRefresh Whether to request a full refresh waveform.
     */
    void updateArea(const BoundingBox& area, bool isFullRefresh) override;

    /** @brief Requests a full-screen GC16 e-ink refresh. */
    void fullRefresh() override;

    /**
     * @brief Returns the framebuffer descriptor used by the controller.
     * @return The open framebuffer file descriptor.
     */
    int fileDescriptor() const;

private:
    int file_descriptor_;
    bool owns_file_descriptor_;

    void issueUpdate(const BoundingBox& area, int waveform) const;
};

#endif
