#ifndef _FEL_CLOCK_H_
#define _FEL_CLOCK_H_

#include "fel_lib.h"

void clock_init_mmc2(const feldev_handle *dev);
void clock_pll_enable_mmc2(const feldev_handle *dev, int enable);
void clock_set_rate_mmc2(const feldev_handle *dev, uint32_t rate);
uint32_t clock_get_rate_mmc2(const feldev_handle *dev);

#endif /* _FEL_CLOCK_H_ */