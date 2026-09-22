#ifndef KINDLE_HAL_EINK_CONTROLLER_H
#define KINDLE_HAL_EINK_CONTROLLER_H

#include "../domain/value_objects.h"

/** @brief Linux framebuffer ioctl used to request an e-ink display update. */
constexpr unsigned long FBIO_EINK_UPDATE_DISPLAY_AREA = 0x46DDUL;

/** @brief Selects the fast partial e-ink update waveform. */
constexpr int fx_update_partial = 0;

/** @brief Selects the full e-ink refresh waveform. */
constexpr int fx_update_full = 1;

/**
 * @brief Controls e-ink display refresh operations.
 *
 * Implementations translate logical display regions into device-specific
 * refresh requests.
 */
class IEinkController {
public:
    /** @brief Releases the e-ink controller resources. */
    virtual ~IEinkController() = default;

    /**
     * @brief Refreshes a rectangular display region.
     * @param area Region of the display to refresh.
     * @param isFullRefresh Whether to use the full-refresh waveform instead of
     *     the fast partial waveform.
     */
    virtual void updateArea(const BoundingBox& area, bool isFullRefresh) = 0;

    /** @brief Performs a full-screen e-ink refresh. */
    virtual void fullRefresh() = 0;
};

#endif
