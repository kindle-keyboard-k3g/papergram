#ifndef KINDLE_TEST_MOCK_EINK_CONTROLLER_H
#define KINDLE_TEST_MOCK_EINK_CONTROLLER_H

#include "../../src/hal/eink_controller.h"

#include <cstddef>
#include <stdexcept>
#include <vector>

class MockEinkController : public IEinkController {
public:
    struct UpdateCall {
        BoundingBox area;
        bool is_full_refresh;
    };

    void updateArea(const BoundingBox& area, bool isFullRefresh) override {
        calls_.push_back({area, isFullRefresh});
    }

    void fullRefresh() override {
        ++full_refresh_count_;
    }

    std::size_t partialUpdateCount() const {
        std::size_t count = 0U;
        for (const UpdateCall& call : calls_) {
            if (!call.is_full_refresh) {
                ++count;
            }
        }
        return count;
    }

    std::size_t areaFullUpdateCount() const {
        std::size_t count = 0U;
        for (const UpdateCall& call : calls_) {
            if (call.is_full_refresh) {
                ++count;
            }
        }
        return count;
    }

    std::size_t fullRefreshCount() const {
        return full_refresh_count_;
    }

    const UpdateCall& callAt(std::size_t index) const {
        return calls_.at(index);
    }

private:
    std::vector<UpdateCall> calls_;
    std::size_t full_refresh_count_ = 0U;
};

#endif
