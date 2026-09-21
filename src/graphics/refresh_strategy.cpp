#include "refresh_strategy.h"
#include "util/debug_log.h"
#include <string>

TypingRefresh::TypingRefresh(IEinkController& controller)
    : controller_(controller) {}

RefreshResult TypingRefresh::refresh(const DirtyTracker& tracker, bool screenChanged) {
    if (screenChanged) {
        DEBUG_LOG("Eink", "TypingRefresh triggered full refresh: screen changed");
        controller_.fullRefresh();
        return RefreshResult::FullGc16;
    }
    if (!tracker.hasDirty()) {
        return RefreshResult::None;
    }
    for (const BoundingBox& box : tracker.regions()) {
        DEBUG_LOG("Eink", "TypingRefresh DU partial update: [" +
                  std::to_string(box.left()) + "," + std::to_string(box.top()) +
                  " - " + std::to_string(box.right()) + "," + std::to_string(box.bottom()) + "]");
        controller_.updateArea(box, false);
    }
    return RefreshResult::PartialDu;
}

FullRefresh::FullRefresh(IEinkController& controller)
    : controller_(controller), keystrokes_(0U) {}

RefreshResult FullRefresh::refresh(const DirtyTracker& tracker, bool screenChanged) {
    if (screenChanged) {
        DEBUG_LOG("Eink", "FullRefresh GC16 full flash: screen changed");
        refreshFully();
        return RefreshResult::FullGc16;
    }
    if (!tracker.hasDirty()) {
        return RefreshResult::None;
    }
    if (reachesLimit()) {
        DEBUG_LOG("Eink", "FullRefresh GC16 full flash: reached keystroke limit");
        refreshFully();
        return RefreshResult::FullGc16;
    }
    for (const BoundingBox& box : tracker.regions()) {
        DEBUG_LOG("Eink", "FullRefresh DU partial update: [" +
                  std::to_string(box.left()) + "," + std::to_string(box.top()) +
                  " - " + std::to_string(box.right()) + "," + std::to_string(box.bottom()) + "]");
        controller_.updateArea(box, false);
    }
    ++keystrokes_;
    return RefreshResult::PartialDu;
}

std::size_t FullRefresh::keystrokesSinceFullRefresh() const {
    return keystrokes_;
}

void FullRefresh::forceFullRefresh() {
    refreshFully();
}

bool FullRefresh::reachesLimit() const {
    return keystrokes_ + 1U >= KEYSTROKE_LIMIT;
}

void FullRefresh::refreshFully() {
    controller_.fullRefresh();
    keystrokes_ = 0U;
}
