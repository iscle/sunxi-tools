#include "fel-clock.h"

#define debug(fmt, ...) printf("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

uint32_t fel_readl(feldev_handle *dev, uint32_t addr);
void fel_writel(feldev_handle *dev, uint32_t addr, uint32_t val);
#define readl(addr)		fel_readl(dev, (addr))
#define writel(val, addr)	fel_writel(dev, (addr), (val))

#define A133_PLL_PERI0_CTRL_REG	(0x03001000 + 0x20)
#define A133_SMHC2_CLK_REG	(0x03001000 + 0x838)
#define A133_SMHC_BGR_REG	(0x03001000 + 0x84c)

#define A133_SMHC_BGR_SMHC2_RST	(1 << 18)
#define A133_SMHC_BGR_SMHC2_GATING	(1 << 2)

void clock_init_mmc2(const feldev_handle *dev)
{
	uint32_t reg_val;

	reg_val = readl(A133_PLL_PERI0_CTRL_REG);
	printf("pll_periph0: 0x%x\n", reg_val);

	/* Enable MMC2 gate */
	reg_val = readl(A133_SMHC_BGR_REG);
	reg_val |= A133_SMHC_BGR_SMHC2_GATING;
	writel(reg_val, A133_SMHC_BGR_REG);

	/* Deassert MMC2 reset */
	reg_val = readl(A133_SMHC_BGR_REG);
	reg_val |= A133_SMHC_BGR_SMHC2_RST;
	writel(reg_val, A133_SMHC_BGR_REG);

	/* Enable PLL */
	writel(1 << 31, A133_SMHC2_CLK_REG);
}

void clock_pll_enable_mmc2(const feldev_handle *dev, int enable)
{
	uint32_t reg_val;
	
	reg_val = readl(A133_SMHC2_CLK_REG);
	if (enable)
		reg_val |= (1 << 31);
	else
		reg_val &= ~(1 << 31);
	writel(reg_val, A133_SMHC2_CLK_REG);
}

static uint32_t clock_get_rate_periph0_1x(const feldev_handle *dev)
{
	uint32_t reg_val;
	uint32_t n;
	uint32_t m1;
	uint32_t m0;

	reg_val = readl(A133_PLL_PERI0_CTRL_REG);
	n = (reg_val >> 8) & 0xff;
	m1 = (reg_val >> 1) & 0x1;
	m0 = (reg_val >> 0) & 0x1;

	return 24000000 * (n + 1) / (m0 + 1) / (m1 + 1) / 2;
}

void clock_set_rate_mmc2(const feldev_handle *dev, uint32_t rate)
{
	uint32_t reg_val;
	uint32_t src;
	uint32_t src_hz;
	uint32_t div;
	uint32_t n;
	uint32_t m;
	uint32_t real_rate;

	if (rate <= 4000000) {
		src = 0; /* OSC24M */
		src_hz = 24000000;
	} else {
		src = 1; /* PLL_PERI0(2x) */
		src_hz = clock_get_rate_periph0_1x(dev) * 2;
	}

	div = (2 * src_hz + rate) / (2 * rate);
	div = (div == 0) ? 1 : div;
	if (div > 128) {
		m = 1;
		n = 0;
	} else if (div > 64) {
		n = 3;
		m = div >> 3;
	} else if (div > 32) {
		n = 2;
		m = div >> 2;
	} else if (div > 16) {
		n = 1;
		m = div >> 1;
	} else {
		n = 0;
		m = div;
	}

	reg_val = (src << 24) | (n << 8) | (m - 1);
	writel(reg_val, A133_SMHC2_CLK_REG);
}

uint32_t clock_get_rate_mmc2(const feldev_handle *dev)
{
	uint32_t reg_val;
	uint32_t src;
	uint32_t src_hz;
	uint32_t n;
	uint32_t m;
	
	reg_val = readl(A133_SMHC2_CLK_REG);
	src = (reg_val >> 24) & 0x3;
	n = (reg_val >> 8) & 0x3;
	m = (reg_val >> 0) & 0xf;

	if (src == 0) { /* OSC24M */
		src_hz = 24000000;
	} else if (src == 1) { /* PLL_PERI0(2x) */
		src_hz = clock_get_rate_periph0_1x(dev) * 2;
	} else {
		printf("Unknown source\n");
		return 0;
	}

	return src_hz / (m + 1) / (1 << n);
}