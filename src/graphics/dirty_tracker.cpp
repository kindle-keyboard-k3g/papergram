#include "dirty_tracker.h"

#include <algorithm>
#include <stdexcept>

void DirtyTracker::mark(const BoundingBox& area) {
    for (std::size_t index = 0U; index < regions_.size(); ++index) {
        if (touches(regions_.at(index), area)) {
            mergeAt(index, area);
            return;
        }
    }
    regions_.push_back(area);
}

void DirtyTracker::add(const BoundingBox& area) {
    mark(area);
}

bool DirtyTracker::hasDirty() const {
    return !regions_.empty();
}

std::size_t DirtyTracker::count() const {
    return regions_.size();
}

BoundingBox DirtyTracker::boundingBox() const {
    if (!hasDirty()) {
        throw std::logic_error("No dirty display area is available");
    }
    BoundingBox result = regions_.front();
    for (std::size_t index = 1U; index < regions_.size(); ++index) {
        result = merged(result, regions_.at(index));
    }
    return result;
}

std::vector<BoundingBox> DirtyTracker::regions() const {
    return regions_;
}

std::vector<BoundingBox> DirtyTracker::consume() {
    std::vector<BoundingBox> result;
    result.swap(regions_);
    return result;
}

void DirtyTracker::clear() {
    regions_.clear();
}

bool DirtyTracker::touches(const BoundingBox& first,
                           const BoundingBox& second) {
    return first.left() <= second.right() + 1 &&
           second.left() <= first.right() + 1 &&
           first.top() <= second.bottom() + 1 &&
           second.top() <= first.bottom() + 1;
}

BoundingBox DirtyTracker::merged(const BoundingBox& first,
                                 const BoundingBox& second) {
    return BoundingBox(std::min(first.left(), second.left()),
                       std::min(first.top(), second.top()),
                       std::max(first.right(), second.right()),
                       std::max(first.bottom(), second.bottom()));
}

void DirtyTracker::mergeAt(std::size_t index, const BoundingBox& area) {
    regions_.at(index) = merged(regions_.at(index), area);
    for (std::size_t other = regions_.size(); other > 0U; --other) {
        const std::size_t candidate = other - 1U;
        if (candidate != index && touches(regions_.at(index), regions_.at(candidate))) {
            regions_.at(index) = merged(regions_.at(index), regions_.at(candidate));
            regions_.erase(regions_.begin() + static_cast<std::ptrdiff_t>(candidate));
            if (candidate < index) {
                --index;
            }
        }
    }
}
