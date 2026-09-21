#ifndef KINDLE_GRAPHICS_DIRTY_TRACKER_H
#define KINDLE_GRAPHICS_DIRTY_TRACKER_H

#include "../domain/value_objects.h"

#include <cstddef>
#include <vector>

class DirtyTracker {
public:
    void mark(const BoundingBox& area);
    void add(const BoundingBox& area);
    bool hasDirty() const;
    std::size_t count() const;
    BoundingBox boundingBox() const;
    std::vector<BoundingBox> regions() const;
    std::vector<BoundingBox> consume();
    void clear();

private:
    std::vector<BoundingBox> regions_;

    static bool touches(const BoundingBox& first, const BoundingBox& second);
    static BoundingBox merged(const BoundingBox& first,
                              const BoundingBox& second);
    void mergeAt(std::size_t index, const BoundingBox& area);
};

#endif
