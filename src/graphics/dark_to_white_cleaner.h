#ifndef KINDLE_GRAPHICS_DARK_TO_WHITE_CLEANER_H
#define KINDLE_GRAPHICS_DARK_TO_WHITE_CLEANER_H

#include "dirty_tracker.h"
#include "../hal/eink_controller.h"

/**
 * @brief Performs a second partial refresh for dark-to-white transitions.
 *
 * The extra direct-update pass reduces residual ghosting on e-ink displays.
 */
class DarkToWhiteCleaner {
public:
    /**
     * @brief Creates a cleaner backed by an e-ink controller.
     *
     * @param controller Controller used to issue partial display updates.
     */
    explicit DarkToWhiteCleaner(IEinkController& controller);

    /**
     * @brief Cleans every tracked dark-to-white region.
     *
     * No update is issued when the tracker is empty.
     *
     * @param darkToWhite Regions whose previous pixels were non-white and whose
     * current pixels are white.
     * @throws Any exception propagated by the e-ink controller.
     */
    void clean(const DirtyTracker& darkToWhite);

private:
    IEinkController& controller_;
};

#endif
