#ifndef KINDLE_GRAPHICS_REFRESH_STRATEGY_H
#define KINDLE_GRAPHICS_REFRESH_STRATEGY_H

#include "dirty_tracker.h"
#include "../hal/eink_controller.h"

#include <cstddef>

class EinkRefreshStrategy {
public:
    virtual ~EinkRefreshStrategy() = default;
    virtual void refresh(const DirtyTracker& tracker,
                         bool screenChanged = false) = 0;
};

class TypingRefresh final : public EinkRefreshStrategy {
public:
    explicit TypingRefresh(IEinkController& controller);
    void refresh(const DirtyTracker& tracker,
                 bool screenChanged = false) override;

private:
    IEinkController& controller_;
};

class FullRefresh final : public EinkRefreshStrategy {
public:
    static constexpr std::size_t KEYSTROKE_LIMIT = 15U;

    explicit FullRefresh(IEinkController& controller);
    void refresh(const DirtyTracker& tracker,
                 bool screenChanged = false) override;
    std::size_t keystrokesSinceFullRefresh() const;

private:
    IEinkController& controller_;
    std::size_t keystrokes_;

    bool reachesLimit() const;
    void refreshFully();
};

#endif
