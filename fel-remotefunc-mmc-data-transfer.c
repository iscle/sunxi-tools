/*
 * Copyright © 2024 Iscle <albertiscle9@gmail.com>
 * 
 * Based on U-Boot code which is:
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Aaron <leafy.myeh@allwinnertech.com>
 */

typedef unsigned int u32;

#define readl(addr)		(*((volatile u32 *)(addr)))
#define writel(v, addr)		(*((volatile u32 *)(addr)) = (u32)(v))

#define SUNXI_MMC_GCTRL_ACCESS_BY_AHB		(0x1 << 31)

#define SUNXI_MMC_RINTSTS_END_BIT_ERROR		(0x1 << 15)
#define SUNXI_MMC_RINTSTS_AUTO_COMMAND_DONE	(0x1 << 14)
#define SUNXI_MMC_RINTSTS_START_BIT_ERROR		(0x1 << 13)
#define SUNXI_MMC_RINTSTS_HARD_WARE_LOCKED		(0x1 << 12)
#define SUNXI_MMC_RINTSTS_FIFO_RUN_ERROR		(0x1 << 11)
#define SUNXI_MMC_RINTSTS_VOLTAGE_CHANGE_DONE	(0x1 << 10)
#define SUNXI_MMC_RINTSTS_DATA_TIMEOUT		(0x1 << 9)
#define SUNXI_MMC_RINTSTS_RESP_TIMEOUT		(0x1 << 8)
#define SUNXI_MMC_RINTSTS_DATA_CRC_ERROR		(0x1 << 7)
#define SUNXI_MMC_RINTSTS_RESP_CRC_ERROR		(0x1 << 6)
#define SUNXI_MMC_RINTSTS_DATA_OVER		(0x1 << 3)
#define SUNXI_MMC_RINTSTS_COMMAND_DONE		(0x1 << 2)
#define SUNXI_MMC_RINTSTS_RESP_ERROR		(0x1 << 1)
#define SUNXI_MMC_RINTSTS_INTERRUPT_ERROR_BIT		\
	(SUNXI_MMC_RINTSTS_RESP_ERROR |			\
	 SUNXI_MMC_RINTSTS_RESP_CRC_ERROR |		\
	 SUNXI_MMC_RINTSTS_DATA_CRC_ERROR |		\
	 SUNXI_MMC_RINTSTS_RESP_TIMEOUT |		\
	 SUNXI_MMC_RINTSTS_DATA_TIMEOUT |		\
	 SUNXI_MMC_RINTSTS_VOLTAGE_CHANGE_DONE |	\
	 SUNXI_MMC_RINTSTS_FIFO_RUN_ERROR |		\
	 SUNXI_MMC_RINTSTS_HARD_WARE_LOCKED |		\
	 SUNXI_MMC_RINTSTS_START_BIT_ERROR |		\
	 SUNXI_MMC_RINTSTS_END_BIT_ERROR)

#define SUNXI_MMC_STATUS_FIFO_LEVEL(reg)	(((reg) >> 17) & 0x3fff)
#define SUNXI_MMC_STATUS_CARD_DATA_BUSY		(0x1 << 9)
#define SUNXI_MMC_STATUS_FIFO_FULL		(0x1 << 3)
#define SUNXI_MMC_STATUS_FIFO_EMPTY		(0x1 << 2)

static void inline __attribute((always_inline)) mmc_trans_data_by_cpu(
	u32 *buff, u32 word_cnt, u32 reading, u32 *mmc_status_reg,
	u32 *mmc_ctrl_reg, u32 *mmc_fifo_reg)
{
	const u32 status_bit = reading ? SUNXI_MMC_STATUS_FIFO_EMPTY :
					 SUNXI_MMC_STATUS_FIFO_FULL;
	u32 i;
	u32 status;

	/* Always read / write data through the CPU */
	writel(readl(mmc_ctrl_reg) | SUNXI_MMC_GCTRL_ACCESS_BY_AHB,
		mmc_ctrl_reg);

	for (i = 0; i < word_cnt;) {
		u32 in_fifo;

		while ((status = readl(mmc_status_reg)) & status_bit);

		/*
		 * For writing we do not easily know the FIFO size, so have
		 * to check the FIFO status after every word written.
		 * TODO: For optimisation we could work out a minimum FIFO
		 * size across all SoCs, and use that together with the current
		 * fill level to write chunks of words.
		 */
		if (!reading) {
			writel(buff[i++], mmc_fifo_reg);
			continue;
		}

		/*
		 * The status register holds the current FIFO level, so we
		 * can be sure to collect as many words from the FIFO
		 * register without checking the status register after every
		 * read. That saves half of the costly MMIO reads, effectively
		 * doubling the read performance.
		 * Some SoCs (A20) report a level of 0 if the FIFO is
		 * completely full (value masked out?). Use a safe minimal
		 * FIFO size in this case.
		 */
		in_fifo = SUNXI_MMC_STATUS_FIFO_LEVEL(status);
		if (in_fifo == 0 && (status & SUNXI_MMC_STATUS_FIFO_FULL))
			in_fifo = 32;
		for (; in_fifo > 0; in_fifo--)
			buff[i++] = readl(mmc_fifo_reg);
		asm volatile ("" : : : "memory"); // dmb
	}
}

static u32 inline __attribute((always_inline)) mmc_rintsts_wait(
	u32 done_bit, u32 *mmc_rintsts_reg)
{
	u32 status;

	do {
		status = readl(mmc_rintsts_reg);
		if (status & SUNXI_MMC_RINTSTS_INTERRUPT_ERROR_BIT)
			return status;
	} while (!(status & done_bit));

	return 0;
}

u32 sunxi_mmc_trans_data(
	u32 *buff, u32 blocks, u32 blocksize, u32 reading, u32 rsp_busy,
	u32 *mmc_rintsts_reg, u32 *mmc_status_reg, u32 *mmc_ctrl_reg,
	u32 *mmc_fifo_reg)
{
	u32 status;
	u32 word_cnt = (blocksize * blocks) >> 2;

	mmc_trans_data_by_cpu(buff, word_cnt, reading, mmc_status_reg, 
			      mmc_ctrl_reg, mmc_fifo_reg);

	status = mmc_rintsts_wait(SUNXI_MMC_RINTSTS_COMMAND_DONE, mmc_rintsts_reg);
	if (status)
		return status;

	status = mmc_rintsts_wait(blocks > 1 ?
		      SUNXI_MMC_RINTSTS_AUTO_COMMAND_DONE :
		      SUNXI_MMC_RINTSTS_DATA_OVER, mmc_rintsts_reg);
	if (status)
		return status;

	if (rsp_busy)
		while (readl(mmc_status_reg) & SUNXI_MMC_STATUS_CARD_DATA_BUSY);

	return 0;
}

