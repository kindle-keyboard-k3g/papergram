#include "refresh_strategy.h"

TypingRefresh::TypingRefresh(IEinkController& controller)
    : controller_(controller) {}

void TypingRefresh::refresh(const DirtyTracker& tracker, bool screenChanged) {
    if (screenChanged) {
        controller_.fullRefresh();
        return;
    }
    if (!tracker.hasDirty()) {
        return;
    }
    controller_.updateArea(tracker.boundingBox(), false);
}

FullRefresh::FullRefresh(IEinkController& controller)
    : controller_(controller), keystrokes_(0U) {}

void FullRefresh::refresh(const DirtyTracker& tracker, bool screenChanged) {
    if (screenChanged) {
        refreshFully();
        return;
    }
    if (!tracker.hasDirty()) {
        return;
    }
    if (reachesLimit()) {
        refreshFully();
        return;
    }
    controller_.updateArea(tracker.boundingBox(), false);
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
