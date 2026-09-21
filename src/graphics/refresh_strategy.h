#ifndef KINDLE_GRAPHICS_REFRESH_STRATEGY_H
#define KINDLE_GRAPHICS_REFRESH_STRATEGY_H

#include "dirty_tracker.h"
#include "../hal/eink_controller.h"

#include <cstddef>

enum class RefreshResult {
    None,
    PartialDu,
    FullGc16
};

class EinkRefreshStrategy {
public:
    virtual ~EinkRefreshStrategy() = default;
    virtual RefreshResult refresh(const DirtyTracker& tracker,
                                  bool screenChanged = false) = 0;
};

class TypingRefresh final : public EinkRefreshStrategy {
public:
    explicit TypingRefresh(IEinkController& controller);
    RefreshResult refresh(const DirtyTracker& tracker,
                          bool screenChanged = false) override;

private:
    IEinkController& controller_;
};

class FullRefresh final : public EinkRefreshStrategy {
public:
    static constexpr std::size_t KEYSTROKE_LIMIT = 15U;

    explicit FullRefresh(IEinkController& controller);
    RefreshResult refresh(const DirtyTracker& tracker,
                          bool screenChanged = false) override;
    std::size_t keystrokesSinceFullRefresh() const;
    void forceFullRefresh();

private:
    IEinkController& controller_;
    std::size_t keystrokes_;

    bool reachesLimit() const;
    void refreshFully();
};

#endif
