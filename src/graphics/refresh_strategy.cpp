#include "refresh_strategy.h"
#include "util/debug_log.h"
#include <string>

TypingRefresh::TypingRefresh(IEinkController& controller)
    : controller_(controller) {}

void TypingRefresh::refresh(const DirtyTracker& tracker, bool screenChanged) {
    if (screenChanged) {
        DEBUG_LOG("Eink", "TypingRefresh triggered full refresh: screen changed");
        controller_.fullRefresh();
        return;
    }
    if (!tracker.hasDirty()) {
        return;
    }
    const BoundingBox box = tracker.boundingBox();
    DEBUG_LOG("Eink", "TypingRefresh DU partial update: [" +
              std::to_string(box.left()) + "," + std::to_string(box.top()) +
              " - " + std::to_string(box.right()) + "," + std::to_string(box.bottom()) + "]");
    controller_.updateArea(box, false);
}

FullRefresh::FullRefresh(IEinkController& controller)
    : controller_(controller), keystrokes_(0U) {}

void FullRefresh::refresh(const DirtyTracker& tracker, bool screenChanged) {
    if (screenChanged) {
        DEBUG_LOG("Eink", "FullRefresh GC16 full flash: screen changed");
        refreshFully();
        return;
    }
    if (!tracker.hasDirty()) {
        return;
    }
    if (reachesLimit()) {
        DEBUG_LOG("Eink", "FullRefresh GC16 full flash: reached keystroke limit");
        refreshFully();
        return;
    }
    const BoundingBox box = tracker.boundingBox();
    DEBUG_LOG("Eink", "FullRefresh DU partial update: [" +
              std::to_string(box.left()) + "," + std::to_string(box.top()) +
              " - " + std::to_string(box.right()) + "," + std::to_string(box.bottom()) + "]");
    controller_.updateArea(box, false);
    ++keystrokes_;
}

std::size_t FullRefresh::keystrokesSinceFullRefresh() const {
    return keystrokes_;
}

bool FullRefresh::reachesLimit() const {
    return keystrokes_ + 1U >= KEYSTROKE_LIMIT;
}

void FullRefresh::refreshFully() {
    controller_.fullRefresh();
    keystrokes_ = 0U;
}
