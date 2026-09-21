#include "dark_to_white_cleaner.h"
#include "util/debug_log.h"
#include <string>

DarkToWhiteCleaner::DarkToWhiteCleaner(IEinkController& controller)
    : controller_(controller) {}

void DarkToWhiteCleaner::clean(const DirtyTracker& darkToWhite) {
    if (!darkToWhite.hasDirty()) {
        return;
    }
    for (const BoundingBox& box : darkToWhite.regions()) {
        DEBUG_LOG("Eink", "DarkToWhiteCleaner 2nd DU pass: [" +
                  std::to_string(box.left()) + "," + std::to_string(box.top()) +
                  " - " + std::to_string(box.right()) + "," + std::to_string(box.bottom()) + "]");
        controller_.updateArea(box, false);
    }
}
