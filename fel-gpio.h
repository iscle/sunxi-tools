/*
 * (C) Copyright 2024 Iscle <albertiscle9@gmail.com>
 */

#ifndef _FEL_GPIO_H_
#define _FEL_GPIO_H_

#include "fel_lib.h"

#define GPIO_PA	0
#define GPIO_PB	1
#define GPIO_PC	2
#define GPIO_PD	3
#define GPIO_PE	4
#define GPIO_PF	5
#define GPIO_PG	6

#define SUN50I_GPC_MMC2	3

void gpio_set_mux(const feldev_handle *dev, uint32_t bank, uint32_t pin, uint32_t val);
void gpio_set_data(const feldev_handle *dev, uint32_t bank, uint32_t pin, uint32_t val);
void gpio_set_dlevel(const feldev_handle *dev, uint32_t bank, uint32_t pin, uint32_t val);
void gpio_set_pull(const feldev_handle *dev, uint32_t bank, uint32_t pin, uint32_t val);

#endif /* _FEL_GPIO_H_ */