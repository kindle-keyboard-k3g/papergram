#ifndef KINDLE_GRAPHICS_DIFF_TRACKER_H
#define KINDLE_GRAPHICS_DIFF_TRACKER_H

#include "dirty_tracker.h"
#include <cstddef>
#include <cstdint>

class BufferDiffTracker {
public:
    static constexpr std::size_t MAX_REGIONS = 64U;

    BufferDiffTracker();

    void compare(const std::uint8_t* previous,
                 const std::uint8_t* current,
                 std::size_t width = 600U,
                 std::size_t height = 800U);

    const DirtyTracker& changed() const;
    const DirtyTracker& darkToWhite() const;
    bool hasChanges() const;
    bool hasDarkToWhite() const;
    void clear();

private:
    DirtyTracker changed_;
    DirtyTracker dark_to_white_;

    void processRow(const std::uint8_t* previous,
                    const std::uint8_t* current,
                    int y,
                    std::size_t width);
    void checkCap(DirtyTracker& tracker, std::size_t width, std::size_t height);
};

#endif
