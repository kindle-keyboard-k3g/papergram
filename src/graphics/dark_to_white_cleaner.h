#ifndef KINDLE_GRAPHICS_DARK_TO_WHITE_CLEANER_H
#define KINDLE_GRAPHICS_DARK_TO_WHITE_CLEANER_H

#include "dirty_tracker.h"
#include "../hal/eink_controller.h"

class DarkToWhiteCleaner {
public:
    explicit DarkToWhiteCleaner(IEinkController& controller);

    void clean(const DirtyTracker& darkToWhite);

private:
    IEinkController& controller_;
};

#endif
