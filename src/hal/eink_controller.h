#ifndef KINDLE_HAL_EINK_CONTROLLER_H
#define KINDLE_HAL_EINK_CONTROLLER_H

#include "../domain/value_objects.h"

constexpr unsigned long FBIO_EINK_UPDATE_DISPLAY_AREA = 0x46DDUL;
constexpr int fx_update_partial = 0;
constexpr int fx_update_full = 1;

class IEinkController {
public:
    virtual ~IEinkController() = default;

    virtual void updateArea(const BoundingBox& area, bool isFullRefresh) = 0;
    virtual void fullRefresh() = 0;
};

#endif
