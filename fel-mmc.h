/*
 * (C) Copyright 2024 Iscle <albertiscle9@gmail.com>
 */

#ifndef _SUNXI_TOOLS_FEL_MMC_H
#define _SUNXI_TOOLS_FEL_MMC_H

#include "fel_lib.h"
#include "progress.h"

void aw_fel_mmc_read(feldev_handle *dev,
			  uint32_t offset, void *buf, size_t len,
			  progress_cb_t progress);
void aw_fel_mmc_write(feldev_handle *dev,
			   uint32_t offset, void *buf, size_t len,
			   progress_cb_t progress);
void aw_fel_mmc_info(feldev_handle *dev);
void aw_fel_mmc_help(void);

#endif
