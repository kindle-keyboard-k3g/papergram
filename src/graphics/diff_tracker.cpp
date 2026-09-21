#include "diff_tracker.h"
#include "../domain/value_objects.h"
#include <algorithm>

BufferDiffTracker::BufferDiffTracker() = default;

void BufferDiffTracker::clear() {
    changed_.clear();
    dark_to_white_.clear();
}

const DirtyTracker& BufferDiffTracker::changed() const {
    return changed_;
}

const DirtyTracker& BufferDiffTracker::darkToWhite() const {
    return dark_to_white_;
}

bool BufferDiffTracker::hasChanges() const {
    return changed_.hasDirty();
}

bool BufferDiffTracker::hasDarkToWhite() const {
    return dark_to_white_.hasDirty();
}

void BufferDiffTracker::compare(const std::uint8_t* previous,
                                const std::uint8_t* current,
                                std::size_t width,
                                std::size_t height) {
    clear();
    if (!previous || !current || width == 0 || height == 0) {
        return;
    }
    for (std::size_t y = 0; y < height; ++y) {
        processRow(previous, current, static_cast<int>(y), width);
    }
    checkCap(changed_, width, height);
    checkCap(dark_to_white_, width, height);
}

void BufferDiffTracker::processRow(const std::uint8_t* previous,
                                   const std::uint8_t* current,
                                   int y,
                                   std::size_t width) {
    const std::size_t row_offset = static_cast<std::size_t>(y) * width;
    const std::uint8_t white_val = GrayscaleColor::WHITE.value();

    int change_start = -1;
    int d2w_start = -1;

    for (std::size_t x = 0; x < width; ++x) {
        const std::uint8_t prev_px = previous[row_offset + x];
        const std::uint8_t curr_px = current[row_offset + x];

        if (prev_px != curr_px) {
            if (change_start < 0) change_start = static_cast<int>(x);
        } else if (change_start >= 0) {
            changed_.mark(BoundingBox(change_start, y, static_cast<int>(x) - 1, y));
            change_start = -1;
        }

        if (prev_px != white_val && curr_px == white_val) {
            if (d2w_start < 0) d2w_start = static_cast<int>(x);
        } else if (d2w_start >= 0) {
            dark_to_white_.mark(BoundingBox(d2w_start, y, static_cast<int>(x) - 1, y));
            d2w_start = -1;
        }
    }

    if (change_start >= 0) {
        changed_.mark(BoundingBox(change_start, y, static_cast<int>(width) - 1, y));
    }
    if (d2w_start >= 0) {
        dark_to_white_.mark(BoundingBox(d2w_start, y, static_cast<int>(width) - 1, y));
    }
}

void BufferDiffTracker::checkCap(DirtyTracker& tracker, std::size_t width, std::size_t height) {
    if (tracker.count() > MAX_REGIONS) {
        tracker.clear();
        tracker.mark(BoundingBox(0, 0, static_cast<int>(width) - 1, static_cast<int>(height) - 1));
    }
}
