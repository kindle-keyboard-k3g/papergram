#ifndef KINDLE_GRAPHICS_REFRESH_STRATEGY_H
#define KINDLE_GRAPHICS_REFRESH_STRATEGY_H

#include "dirty_tracker.h"
#include "../hal/eink_controller.h"

#include <cstddef>

/** @brief Result of an e-ink refresh strategy operation. */
enum class RefreshResult {
    /** @brief No refresh was required. */
    None,

    /** @brief One or more direct-update partial refreshes were issued. */
    PartialDu,

    /** @brief A full GC16 refresh was issued. */
    FullGc16
};

/**
 * @brief Abstract policy for converting dirty regions into e-ink updates.
 */
class EinkRefreshStrategy {
public:
    /** @brief Releases the refresh strategy. */
    virtual ~EinkRefreshStrategy() = default;

    /**
     * @brief Applies the strategy to the current dirty regions.
     *
     * @param tracker Regions that need display updates.
     * @param screenChanged Whether the displayed screen changed completely.
     * @return The type of refresh that was performed.
     * @throws Any exception propagated by the configured e-ink controller.
     */
    virtual RefreshResult refresh(const DirtyTracker& tracker,
                                  bool screenChanged = false) = 0;
};

/**
 * @brief Uses direct-update partial refreshes for typing-oriented updates.
 *
 * A screen transition escalates to a full GC16 refresh.
 */
class TypingRefresh final : public EinkRefreshStrategy {
public:
    /**
     * @brief Creates a typing refresh strategy.
     *
     * @param controller Controller used to issue e-ink updates.
     */
    explicit TypingRefresh(IEinkController& controller);

    /**
     * @brief Refreshes dirty regions, or the full screen after a transition.
     *
     * @param tracker Regions that need display updates.
     * @param screenChanged Whether the displayed screen changed completely.
     * @return FullGc16 for a screen transition, PartialDu for dirty regions, or
     * None when no update is needed.
     * @throws Any exception propagated by the e-ink controller.
     */
    RefreshResult refresh(const DirtyTracker& tracker,
                          bool screenChanged = false) override;

private:
    IEinkController& controller_;
};

/**
 * @brief Uses partial updates until a keystroke limit requires a full refresh.
 */
class FullRefresh final : public EinkRefreshStrategy {
public:
    /** @brief Number of partial-update keystrokes before a full refresh. */
    static constexpr std::size_t KEYSTROKE_LIMIT = 15U;

    /**
     * @brief Creates a full-refresh strategy with a zeroed keystroke count.
     *
     * @param controller Controller used to issue e-ink updates.
     */
    explicit FullRefresh(IEinkController& controller);

    /**
     * @brief Refreshes dirty regions and periodically escalates to GC16.
     *
     * A screen transition or the next update reaching KEYSTROKE_LIMIT causes a
     * full refresh and resets the partial-update count.
     *
     * @param tracker Regions that need display updates.
     * @param screenChanged Whether the displayed screen changed completely.
     * @return FullGc16 when a full refresh is performed, PartialDu for a partial
     * update, or None when no update is needed.
     * @throws Any exception propagated by the e-ink controller.
     */
    RefreshResult refresh(const DirtyTracker& tracker,
                          bool screenChanged = false) override;

    /**
     * @brief Returns the number of partial updates since the last full refresh.
     *
     * @return Current partial-update keystroke count.
     */
    std::size_t keystrokesSinceFullRefresh() const;

    /**
     * @brief Performs and records an immediate full GC16 refresh.
     *
     * @throws Any exception propagated by the e-ink controller.
     */
    void forceFullRefresh();

private:
    IEinkController& controller_;
    std::size_t keystrokes_;

    bool reachesLimit() const;
    void refreshFully();
};

#endif
