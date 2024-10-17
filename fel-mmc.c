/*
 * (C) Copyright 2024 Iscle <albertiscle9@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fel_lib.h"
#include "progress.h"
#include "fel-gpio.h"
#include "fel-i2c.h"

#include "fel-remotefunc-mmc-data-transfer.h"

#define debug(fmt, ...) printf("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
/*****************************************************************************/

// typedef struct {
// 	uint32_t  id;
// 	uint8_t   write_enable_cmd;
// 	uint8_t   large_erase_cmd;
// 	uint32_t  large_erase_size;
// 	uint8_t   small_erase_cmd;
// 	uint32_t  small_erase_size;
// 	uint8_t   program_cmd;
// 	uint32_t  program_size;
// 	char     *text_description;
// } spi_flash_info_t;

// spi_flash_info_t spi_flash_info[] = {
// 	{ .id = 0xEF40, .write_enable_cmd = 0x6,
// 	  .large_erase_cmd = 0xD8, .large_erase_size = 64 * 1024,
// 	  .small_erase_cmd = 0x20, .small_erase_size =  4 * 1024,
// 	  .program_cmd = 0x02, .program_size = 256,
// 	  .text_description = "Winbond W25Qxx" },
// 	{ .id = 0xC220, .write_enable_cmd = 0x6,
// 	  .large_erase_cmd = 0xD8, .large_erase_size = 64 * 1024,
// 	  .small_erase_cmd = 0x20, .small_erase_size =  4 * 1024,
// 	  .program_cmd = 0x02, .program_size = 256,
// 	  .text_description = "Macronix MX25Lxxxx" },
// 	{ .id = 0x1C70, .write_enable_cmd = 0x6,
// 	  .large_erase_cmd = 0xD8, .large_erase_size = 64 * 1024,
// 	  .small_erase_cmd = 0x20, .small_erase_size =  4 * 1024,
// 	  .program_cmd = 0x02, .program_size = 256,
// 	  .text_description = "Eon EN25QHxx" },
// };

// spi_flash_info_t default_spi_flash_info = {
// 	.id = 0x0000, .write_enable_cmd = 0x6,
// 	.large_erase_cmd = 0xD8, .large_erase_size = 64 * 1024,
// 	.small_erase_cmd = 0x20, .small_erase_size =  4 * 1024,
// 	.program_cmd = 0x02, .program_size = 256,
// 	.text_description = "Unknown",
// };

/*****************************************************************************/

uint32_t fel_readl(feldev_handle *dev, uint32_t addr);
void fel_writel(feldev_handle *dev, uint32_t addr, uint32_t val);
#define readl(addr)		fel_readl(dev, (addr))
#define writel(val, addr)	fel_writel(dev, (addr), (val))

/* CCU */
#define A133_CCU_SMHC2_CLK_REG	(0x03001000 + 0x838)
#define A133_CCU_SMHC_BGR_REG	(0x03001000 + 0x84c)

#define A133_CCU_SMHC_BGR_SMHC2_RST	(1 << 18)
#define A133_CCU_SMHC_BGR_SMHC2_GATING	(1 << 2)

/* MMC */
#define SUNXI_MMC_CTRL		(mmc_base(dev) + 0x00)
#define SUNXI_MMC_CLKDIV	(mmc_base(dev) + 0x04)
#define SUNXI_MMC_TMOUT		(mmc_base(dev) + 0x08)
#define SUNXI_MMC_CTYPE		(mmc_base(dev) + 0x0c)
#define SUNXI_MMC_BLKSIZ	(mmc_base(dev) + 0x10)
#define SUNXI_MMC_BYTCNT	(mmc_base(dev) + 0x14)
#define SUNXI_MMC_CMD		(mmc_base(dev) + 0x18)
#define SUNXI_MMC_CMDARG	(mmc_base(dev) + 0x1c)
#define SUNXI_MMC_RESP0		(mmc_base(dev) + 0x20)
#define SUNXI_MMC_RESP1		(mmc_base(dev) + 0x24)
#define SUNXI_MMC_RESP2		(mmc_base(dev) + 0x28)
#define SUNXI_MMC_RESP3		(mmc_base(dev) + 0x2c)
#define SUNXI_MMC_RINTSTS	(mmc_base(dev) + 0x38)
#define SUNXI_MMC_STATUS	(mmc_base(dev) + 0x3c)
#define SUNXI_MMC_FIFOTH	(mmc_base(dev) + 0x40)
#define SUNXI_MMC_FUNS		(mmc_base(dev) + 0x44)
#define SUNXI_MMC_DBGC		(mmc_base(dev) + 0x50)
#define SUNXI_MMC_CSDC		(mmc_base(dev) + 0x54)
#define SUNXI_MMC_NTSR		(mmc_base(dev) + 0x5c)
#define SUNXI_MMC_HWRST		(mmc_base(dev) + 0x78)
#define SUNXI_MMC_THLD		(mmc_base(dev) + 0x100)
#define SUNXI_MMC_DRV_DL	(mmc_base(dev) + 0x140)
#define SUNXI_MMC_SAMP_DL	(mmc_base(dev) + 0x144)
#define SUNXI_MMC_FIFO		(mmc_base(dev) + 0x200)

#define SUNXI_MMC_CTRL_DMA_RST	(1 << 2)
#define SUNXI_MMC_CTRL_FIFO_RST	(1 << 1)
#define SUNXI_MMC_CTRL_SOFT_RST	(1 << 0)

#define SUNXI_MMC_CLKDIV_CLK_EN		(1 << 16)
#define SUNXI_MMC_CLKDIV_CLK_DIV_MASK	(0xff << 0)

#define SUNXI_MMC_CMD_CMD_LOAD		(1 << 31)
#define SUNXI_MMC_CMD_PRG_CLK		(1 << 21)
#define SUNXI_MMC_CMD_SEND_INIT_SEQ	(1 << 15)
#define SUNXI_MMC_CMD_WAIT_PRE_OVER	(1 << 13)
#define SUNXI_MMC_CMD_AUTO_STOP		(1 << 12)
#define SUNXI_MMC_CMD_WRITE		(1 << 10)
#define SUNXI_MMC_CMD_DATA_EXPIRE	(1 << 9)
#define SUNXI_MMC_CMD_CHK_RESPONSE_CRC	(1 << 8)
#define SUNXI_MMC_CMD_LONG_RESPONSE	(1 << 7)
#define SUNXI_MMC_CMD_RESP_EXPIRE	(1 << 6)

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

#define SUNXI_MMC_STATUS_CARD_DATA_BUSY	(0x1 << 9)

#define SUNXI_MMC_NTSR_MODE_SELEC	(1 << 31)

#define SUNXI_MMC_SAMP_DL_SW_EN		(1 << 7)

#define MMC_DATA_READ		1
#define MMC_DATA_WRITE		2

#define MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SET_DSR			4
#define MMC_CMD_SWITCH			6
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_EXT_CSD		8
#define MMC_CMD_SEND_CSD		9
#define MMC_CMD_SEND_CID		10
#define MMC_CMD_STOP_TRANSMISSION	12
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define MMC_CMD_SEND_TUNING_BLOCK		19
#define MMC_CMD_SEND_TUNING_BLOCK_HS200	21
#define MMC_CMD_SET_BLOCK_COUNT         23
#define MMC_CMD_WRITE_SINGLE_BLOCK	24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_ERASE_GROUP_START	35
#define MMC_CMD_ERASE_GROUP_END		36
#define MMC_CMD_ERASE			38
#define MMC_CMD_APP_CMD			55
#define MMC_CMD_SPI_READ_OCR		58
#define MMC_CMD_SPI_CRC_ON_OFF		59
#define MMC_CMD_RES_MAN			62

#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_PRESENT 1

#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
			MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

static uint32_t mmc_base(feldev_handle *dev)
{
	soc_info_t *soc_info = dev->soc_info;
	switch (soc_info->soc_id) {
	case 0x1855: /* A133 */
		return 0x04022000; /* MMC2 */
	default: /* Unknown/Unsupported SoC */
		printf("MMC support not implemented yet for %x (%s)!\n",
		       soc_info->soc_id, soc_info->name);
		return 0;
	}
}

static int mmc_update_clk(feldev_handle *dev)
{
	uint32_t reg_val;
	int cnt;

	reg_val = readl(SUNXI_MMC_CLKDIV);
	reg_val |= (1 << 31); /* MASK_DATA0 */
	writel(reg_val, SUNXI_MMC_CLKDIV);

	reg_val = SUNXI_MMC_CMD_CMD_LOAD |
		  SUNXI_MMC_CMD_PRG_CLK |
		  SUNXI_MMC_CMD_WAIT_PRE_OVER;
	writel(reg_val, SUNXI_MMC_CMD);

	cnt = 0xff;
	while (readl(SUNXI_MMC_CMD) & SUNXI_MMC_CMD_CMD_LOAD) {
		if (cnt-- == 0) {
			debug("MMC timeout\n");
			return -1;
		}
	}

	reg_val = readl(SUNXI_MMC_CLKDIV);
	reg_val &= ~(1 << 31); /* MASK_DATA0 */
	writel(reg_val, SUNXI_MMC_CLKDIV);

	/* Clear interrupt flag */
	reg_val = readl(SUNXI_MMC_RINTSTS);
	debug("RINTSTS: 0x%08x\n", reg_val);
	writel(reg_val, SUNXI_MMC_RINTSTS);

	return 0;
}

static int mmc_core_init(const feldev_handle *dev)
{
	int cnt;
	uint32_t reg_val;

	/* Reset MMC controller */
	writel(SUNXI_MMC_CTRL_DMA_RST | SUNXI_MMC_CTRL_FIFO_RST |
		SUNXI_MMC_CTRL_SOFT_RST, SUNXI_MMC_CTRL);

	cnt = 0xff;
	while (readl(SUNXI_MMC_CTRL) & (SUNXI_MMC_CTRL_DMA_RST |
		SUNXI_MMC_CTRL_FIFO_RST | SUNXI_MMC_CTRL_SOFT_RST)) {
		if (cnt-- == 0) {
			debug("MMC timeout\n");
			return -1;
		}
	}

	// /* Set FIFO access mode */
	// reg_val = readl(SUNXI_MMC_CTRL);
	// reg_val |= (1 << 31); /* AHB bus */
	// writel(reg_val, SUNXI_MMC_CTRL);

	// /* Set data and response timeout value */
	// writel(0xffffffff, SUNXI_MMC_TMOUT);
	// writel((512 << 16) | (1 << 2) | (1 << 0), SUNXI_MMC_THLD);
	// writel(3, SUNXI_MMC_CSDC);
	// writel(0xdeb, SUNXI_MMC_DBGC);

	/* Release eMMC reset signal */
	writel(1, SUNXI_MMC_HWRST);
	writel(0, SUNXI_MMC_HWRST);
	writel(1, SUNXI_MMC_HWRST);
}

static void mmc_config_delay(const feldev_handle *dev, uint32_t clock)
{
	uint32_t reg_val;

	reg_val = readl(SUNXI_MMC_DRV_DL);
	reg_val &= ~0x3f;
	reg_val |= 0x1f;
	writel(reg_val, SUNXI_MMC_DRV_DL);
}

static int mmc_config_clock_modex(const feldev_handle *dev, uint32_t clock)
{
	int ret;
	uint32_t reg_val;
	uint32_t real_clock;
	int cnt;

	clock_pll_enable_mmc2(dev, 0);

	clock_set_rate_mmc2(dev, clock * 2);
	real_clock = clock_get_rate_mmc2(dev);
	printf("MMC2 clock: requested = %d, real = %d\n", clock * 2, real_clock);

	clock_pll_enable_mmc2(dev, 1);

	reg_val = readl(SUNXI_MMC_CLKDIV);
	reg_val &= ~SUNXI_MMC_CLKDIV_CLK_DIV_MASK;
	// reg_val |= 1;
	writel(reg_val, SUNXI_MMC_CLKDIV);

	ret = mmc_update_clk(dev);
	if (ret)
		return ret;

	mmc_config_delay(dev, clock);

	return 0;
}

static int mmc_config_clock(const feldev_handle *dev, uint32_t clock)
{
	int ret;
	uint32_t reg_val;
	int cnt;

	/* Disable card clock */
	reg_val = readl(SUNXI_MMC_CLKDIV);
	reg_val &= ~SUNXI_MMC_CLKDIV_CLK_EN;
	writel(reg_val, SUNXI_MMC_CLKDIV);

	ret = mmc_update_clk(dev);
	if (ret)
		return ret;

	ret = mmc_config_clock_modex(dev, clock);
	if (ret)
		return ret;

	/* Enable card clock */
	reg_val = readl(SUNXI_MMC_CLKDIV);
	reg_val |= SUNXI_MMC_CLKDIV_CLK_EN;
	writel(reg_val, SUNXI_MMC_CLKDIV);

	ret = mmc_update_clk(dev);
	if (ret)
		return ret;

	return 0;
}

static int mmc_set_ios(const feldev_handle *dev, uint32_t clock, uint32_t bus_width, uint32_t speed_mode)
{
	uint32_t reg_val;
	int cnt;

	if (clock) {
		// TODO
	}

	switch (bus_width) {
	case 1:
		writel(0, SUNXI_MMC_CTYPE);
		break;
	case 4:
		writel(1, SUNXI_MMC_CTYPE);
		break;
	case 8:
		writel(2, SUNXI_MMC_CTYPE);
		break;
	default:
		printf("Unsupported bus width %d\n", bus_width);
		return -1;
	}

	// TODO: Implement DDR speed mode

	return 0;
}

/*
 * Init the MMC controller and setup pins muxing.
 */
static bool mmc_init(feldev_handle *dev)
{
	int cnt;
	uint32_t reg_val;
	soc_info_t *soc_info = dev->soc_info;
	if (!soc_info) {
		printf("Unable to fetch device information. "
		       "Possibly unknown device.\n");
		return false;
	}

	debug("Initializing I2C controller for %x (%s)...\n",
	      soc_info->soc_id, soc_info->name);
	
	// i2c_init(dev);

	// debug("Initializing AXP controller for %x (%s)...\n",
	//       soc_info->soc_id, soc_info->name);

	// axp_init(dev);

	debug("Initializing MMC controller for %x (%s)...\n",
	      soc_info->soc_id, soc_info->name);

	debug("Configuring MMC pins muxing...\n");

	/* Setup MMC pins muxing */
	switch (soc_info->soc_id) {
	case 0x1855: /* Allwinner A133 */
		gpio_set_mux(dev, GPIO_PC, 0, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 1, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 5, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 6, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 8, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 9, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 10, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 11, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 13, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 14, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 15, SUN50I_GPC_MMC2);
		gpio_set_mux(dev, GPIO_PC, 16, SUN50I_GPC_MMC2);
		gpio_set_dlevel(dev, GPIO_PC, 0, 3);
		gpio_set_dlevel(dev, GPIO_PC, 1, 3);
		gpio_set_dlevel(dev, GPIO_PC, 5, 3);
		gpio_set_dlevel(dev, GPIO_PC, 6, 3);
		gpio_set_dlevel(dev, GPIO_PC, 8, 3);
		gpio_set_dlevel(dev, GPIO_PC, 9, 3);
		gpio_set_dlevel(dev, GPIO_PC, 10, 3);
		gpio_set_dlevel(dev, GPIO_PC, 11, 3);
		gpio_set_dlevel(dev, GPIO_PC, 13, 3);
		gpio_set_dlevel(dev, GPIO_PC, 14, 3);
		gpio_set_dlevel(dev, GPIO_PC, 15, 3);
		gpio_set_dlevel(dev, GPIO_PC, 16, 3);
		gpio_set_pull(dev, GPIO_PC, 0, 2);
		gpio_set_pull(dev, GPIO_PC, 1, 1);
		gpio_set_pull(dev, GPIO_PC, 5, 1);
		gpio_set_pull(dev, GPIO_PC, 6, 1);
		gpio_set_pull(dev, GPIO_PC, 8, 1);
		gpio_set_pull(dev, GPIO_PC, 9, 1);
		gpio_set_pull(dev, GPIO_PC, 10, 1);
		gpio_set_pull(dev, GPIO_PC, 11, 1);
		gpio_set_pull(dev, GPIO_PC, 13, 1);
		gpio_set_pull(dev, GPIO_PC, 14, 1);
		gpio_set_pull(dev, GPIO_PC, 15, 1);
		gpio_set_pull(dev, GPIO_PC, 16, 1);
		break;
	default: /* Unknown/Unsupported SoC */
		printf("MMC support not implemented yet for %x (%s)!\n",
		       soc_info->soc_id, soc_info->name);
		return false;
	}

	debug("Initializing MMC clock...\n");
	clock_init_mmc2(dev);

	debug("Initializing MMC controller core...\n");
	mmc_core_init(dev);

	/* Set 8-bit bus width */
	writel(0x2, SUNXI_MMC_CTYPE);

	mmc_config_clock(dev, 25000000);

	debug("Setting MMC bus width...\n");

	debug("MMC controller initialized.\n");

	return true;
}

static uint32_t mmc_rint_wait(feldev_handle *dev, uint done_bit)
{
	uint32_t status;
	int cnt;

	cnt = 0xff;
	do {
		status = readl(SUNXI_MMC_RINTSTS);
		if (status & SUNXI_MMC_RINTSTS_INTERRUPT_ERROR_BIT) {
			debug("MMC error: 0x%08x\n", status);
			return status;
		}
		if (cnt-- == 0) {
			debug("MMC timeout\n");
			return -1;
		}
	} while (!(status & done_bit));

	return 0;
}

static uint32_t sunxi_mmc_send_cmd(feldev_handle *dev,
	/* mmc_cmd */
	uint16_t cmdidx, uint32_t resp_type, uint32_t cmdarg, uint32_t response[4],
	/* mmc_data */
	char *buf, uint32_t flags, uint32_t blocks, uint32_t blocksize)
{
	soc_info_t *soc_info = dev->soc_info;
	unsigned int cmdval = SUNXI_MMC_CMD_CMD_LOAD;
	unsigned int status = 0;
	unsigned int bytecnt = 0;

	uint32_t reg_val;

	reg_val = readl(SUNXI_MMC_CTRL);
	debug("MMC_CTRL: 0x%08x\n", reg_val);
	reg_val = readl(SUNXI_MMC_CLKDIV);
	debug("MMC_CLKDIV: 0x%08x\n", reg_val);
	reg_val = readl(SUNXI_MMC_CTYPE);
	debug("MMC_CTYPE: 0x%08x\n", reg_val);
	reg_val = readl(SUNXI_MMC_STATUS);
	debug("MMC_STATUS: 0x%08x\n", reg_val);
	reg_val = readl(SUNXI_MMC_CSDC);
	debug("MMC_CSDC: 0x%08x\n", reg_val);
	
	if (cmdidx == 12)
		return 0;

	if (!cmdidx)
		cmdval |= SUNXI_MMC_CMD_SEND_INIT_SEQ;
	if (resp_type & MMC_RSP_PRESENT)
		cmdval |= SUNXI_MMC_CMD_RESP_EXPIRE;
	if (resp_type & MMC_RSP_136)
		cmdval |= SUNXI_MMC_CMD_LONG_RESPONSE;
	if (resp_type & MMC_RSP_CRC)
		cmdval |= SUNXI_MMC_CMD_CHK_RESPONSE_CRC;

	if (buf) {
		cmdval |= SUNXI_MMC_CMD_DATA_EXPIRE |
				SUNXI_MMC_CMD_WAIT_PRE_OVER;
		if (flags & MMC_DATA_WRITE)
			cmdval |= SUNXI_MMC_CMD_WRITE;
		if (blocks > 1)
			cmdval |= SUNXI_MMC_CMD_AUTO_STOP;
		writel(blocksize, SUNXI_MMC_BLKSIZ);
		writel(blocks * blocksize, SUNXI_MMC_BYTCNT);
	}


	debug("cmd %d(0x%08x), arg 0x%08x\n",
	      cmdidx, cmdval | cmdidx, cmdarg);
	writel(cmdarg, SUNXI_MMC_CMDARG);

	if (!buf)
		writel(cmdval | cmdidx, SUNXI_MMC_CMD);

	if (buf) {

		/*
		* transfer data and check status
		* STATREG[2] : FIFO empty
		* STATREG[3] : FIFO full
		*/
		bytecnt = blocksize * blocks;
		debug("trans data %d bytes\n", bytecnt);
		writel(cmdval | cmdidx, SUNXI_MMC_CMD);
		aw_fel_remotefunc_prepare_sunxi_mmc_trans_data(dev,
			soc_info->spl_addr, blocks, blocksize,
			!!(flags & MMC_DATA_READ), !!(resp_type & MMC_RSP_BUSY),
			SUNXI_MMC_RINTSTS, SUNXI_MMC_STATUS, SUNXI_MMC_CTRL,
			SUNXI_MMC_FIFO);
		aw_fel_remotefunc_execute(dev, &status);
		if (status)
			return status;
		aw_fel_read(dev, soc_info->spl_addr, buf, bytecnt);
	} else {
		status = mmc_rint_wait(dev, SUNXI_MMC_RINTSTS_COMMAND_DONE);
		if (status)
			return status;

		if (resp_type & MMC_RSP_BUSY)
			while (readl(SUNXI_MMC_STATUS) &
				SUNXI_MMC_STATUS_CARD_DATA_BUSY);
	}

	if (resp_type & MMC_RSP_136) {
		response[0] = readl(SUNXI_MMC_RESP3);
		response[1] = readl(SUNXI_MMC_RESP2);
		response[2] = readl(SUNXI_MMC_RESP1);
		response[3] = readl(SUNXI_MMC_RESP0);
		debug("mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
		      response[3], response[2],
		      response[1], response[0]);
	} else {
		response[0] = readl(SUNXI_MMC_RESP0);
		debug("mmc resp 0x%08x\n", response[0]);
	}

	writel(0xffffffff, SUNXI_MMC_RINTSTS);
	writel(readl(SUNXI_MMC_CTRL) | SUNXI_MMC_CTRL_FIFO_RST,
	       SUNXI_MMC_CTRL);

	return 0;
}

static uint32_t sunxi_mmc_send_cmd_retry(feldev_handle *dev,
	/* mmc_cmd */
	uint16_t cmdidx, uint32_t resp_type, uint32_t cmdarg, uint32_t response[4],
	/* mmc_data */
	char *buf, uint32_t flags, uint32_t blocks, uint32_t blocksize,
	uint32_t retries)
{
	uint32_t status;

	do {
		status = sunxi_mmc_send_cmd(dev, cmdidx, resp_type, cmdarg,
					    response, buf, flags, blocks, blocksize);
	} while (status && retries--);

	return status;
}

/*
 * Backup the initial portion of the SRAM, which can be used as
 * a temporary data buffer.
 */
static void *backup_sram(feldev_handle *dev)
{
	soc_info_t *soc_info = dev->soc_info;
	size_t bufsize = soc_info->scratch_addr - soc_info->spl_addr;
	void *buf = malloc(bufsize);
	aw_fel_read(dev, soc_info->spl_addr, buf, bufsize);
	return buf;
}

/*
 * Restore the initial portion of the SRAM, which can be used as
 * a temporary data buffer.
 */
static void restore_sram(feldev_handle *dev, void *buf)
{
	soc_info_t *soc_info = dev->soc_info;
	size_t bufsize = soc_info->scratch_addr - soc_info->spl_addr;
	aw_fel_write(dev, buf, soc_info->spl_addr, bufsize);
	free(buf);
}

void aw_fel_mmc_read(feldev_handle *dev,
			  uint32_t offset, void *buf, size_t len,
			  progress_cb_t progress)
{

}

void aw_fel_mmc_write(feldev_handle *dev,
			   uint32_t offset, void *buf, size_t len,
			   progress_cb_t progress)
{
	
}

void aw_fel_mmc_info(feldev_handle *dev)
{
	uint32_t response[4];
	uint32_t status;
	void *backup = backup_sram(dev);

	if (!mmc_init(dev))
		return;

	/* Go idle */
	status = sunxi_mmc_send_cmd(dev,
		MMC_CMD_GO_IDLE_STATE, MMC_RSP_NONE, 0, response,
		NULL, 0, 0, 0);
	if (status) {
		printf("MMC init failed (0x%x)\n", status);
		return;
	}

	status = sunxi_mmc_send_cmd(dev,
		MMC_CMD_SEND_OP_COND, MMC_RSP_R3, 0, response,
		NULL, 0, 0, 0);
	if (status) {
		printf("MMC init failed (0x%x)\n", status);
		return;
	}

	status = sunxi_mmc_send_cmd_retry(dev,
		MMC_CMD_ALL_SEND_CID, MMC_RSP_R2, 0, response,
		NULL, 0, 0, 0,
		4);
	if (status) {
		printf("MMC info failed (0x%x)\n", status);
		return;
	}

	restore_sram(dev, backup);
}

/*
 * Show a help message about the available "mmc-*" commands.
 */
void aw_fel_mmc_help(void)
{
	printf("	mmc-info			Retrieves basic information\n"
	       "	mmc-read addr length file	Write MMC contents into file\n"
	       "	mmc-write addr file		Store file contents into MMC\n");
}
