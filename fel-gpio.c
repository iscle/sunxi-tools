/*
 * (C) Copyright 2024 Iscle <albertiscle9@gmail.com>
 */

#include "fel-gpio.h"
#include <stdio.h>

uint32_t fel_readl(feldev_handle *dev, uint32_t addr);
void fel_writel(feldev_handle *dev, uint32_t addr, uint32_t val);
#define readl(addr)		fel_readl(dev, (addr))
#define writel(val, addr)	fel_writel(dev, (addr), (val))

#define BANK_MEM_SIZE		0x24
#define MUX_REGS_OFFSET		0x0
#define MUX_FIELD_WIDTH		4
#define DATA_REGS_OFFSET	0x10
#define DATA_FIELD_WIDTH	1
#define DLEVEL_REGS_OFFSET	0x14
#define DLEVEL_FIELD_WIDTH	2
#define PULL_REGS_OFFSET	0x1c
#define PULL_FIELD_WIDTH	2

#define D1_BANK_MEM_SIZE	0x30
#define D1_DLEVEL_FIELD_WIDTH	4
#define D1_PULL_REGS_OFFSET	0x24

#define PINS_PER_BANK		32

static uint32_t get_gpio_base(const feldev_handle *dev)
{
	switch (dev->soc_info->soc_id) {
	case 0x1728: /* H6 */
	case 0x1816: /* V536 */
	case 0x1817: /* V831 */
	case 0x1823: /* H616 */
	case 0x1855: /* A133 */
		return 0x0300b000;
	default:
		printf("fel-gpio: get_gpio_base: Unknown SoC ID: %08x\n",
			dev->soc_info->soc_id);
		return 0;
	}
}

static uint32_t get_bank_mem_size(const feldev_handle *dev)
{
	switch (dev->soc_info->soc_id) {
	case 0x1855: /* A133 */
		return BANK_MEM_SIZE;
	default:
		printf("fel-gpio: get_bank_mem_size: Unknown SoC ID: %08x\n",
			dev->soc_info->soc_id);
		return BANK_MEM_SIZE;
	}
}

static uint32_t get_dlevel_field_width(const feldev_handle *dev)
{
	switch (dev->soc_info->soc_id) {
	case 0x1855: /* A133 */
		return DLEVEL_FIELD_WIDTH;
	default:
		printf("fel-gpio: get_dlevel_field_width: Unknown SoC ID: %08x\n",
			dev->soc_info->soc_id);
		return DLEVEL_FIELD_WIDTH;
	}
}

static uint32_t get_pull_regs_offset(const feldev_handle *dev)
{
	switch (dev->soc_info->soc_id) {
	case 0x1855: /* A133 */
		return PULL_REGS_OFFSET;
	default:
		printf("fel-gpio: get_pull_regs_offset: Unknown SoC ID: %08x\n",
			dev->soc_info->soc_id);
		return PULL_REGS_OFFSET;
	}
}

/*
 * The sunXi PIO registers are organized as a series of banks, with registers
 * for each bank in the following order:
 *  - Mux config
 *  - Data value
 *  - Drive level
 *  - Pull direction
 *
 * Multiple consecutive registers are used for fields wider than one bit.
 *
 * The following functions calculate the register and the bit offset to access.
 * They take a pin number which is relative to the start of the current device.
 */
static void sunxi_mux_reg(const feldev_handle *dev, uint32_t bank,
	uint32_t pin, uint32_t *reg, uint32_t *shift, uint32_t *mask)
{
	uint32_t offset = pin % PINS_PER_BANK * MUX_FIELD_WIDTH;

	*reg   = get_gpio_base(dev) + bank * get_bank_mem_size(dev) +
		 MUX_REGS_OFFSET + offset / 32 * 4;
	*shift = offset % 32;
	*mask  = ((1 << MUX_FIELD_WIDTH) - 1) << *shift;
}

static void sunxi_data_reg(const feldev_handle *dev, uint32_t bank,
	uint32_t pin, uint32_t *reg, uint32_t *shift, uint32_t *mask)
{
	uint32_t offset = pin % PINS_PER_BANK * DATA_FIELD_WIDTH;

	*reg   = get_gpio_base(dev) + bank * get_bank_mem_size(dev) +
		 DATA_REGS_OFFSET + offset / 32 * 4;
	*shift = offset % 32;
	*mask  = ((1 << DATA_FIELD_WIDTH) - 1) << *shift;
}

static void sunxi_dlevel_reg(const feldev_handle *dev, uint32_t bank,
	uint32_t pin, uint32_t *reg, uint32_t *shift, uint32_t *mask)
{
	uint32_t offset = pin % PINS_PER_BANK * get_dlevel_field_width(dev);

	*reg   = get_gpio_base(dev) + bank * get_bank_mem_size(dev) +
		 DLEVEL_REGS_OFFSET + offset / 32 * 4;
	*shift = offset % 32;
	*mask  = ((1 << get_dlevel_field_width(dev)) - 1) << *shift;
}

static void sunxi_pull_reg(const feldev_handle *dev, uint32_t bank,
	uint32_t pin, uint32_t *reg, uint32_t *shift, uint32_t *mask)
{
	uint32_t offset = pin % PINS_PER_BANK * PULL_FIELD_WIDTH;

	*reg   = get_gpio_base(dev) + bank * get_bank_mem_size(dev) +
		 get_pull_regs_offset(dev) + offset / 32 * 4;
	*shift = offset % 32;
	*mask  = ((1 << PULL_FIELD_WIDTH) - 1) << *shift;
}

void gpio_set_mux(const feldev_handle *dev, uint32_t bank, uint32_t pin, uint32_t val)
{
	uint32_t reg, shift, mask;

	sunxi_mux_reg(dev, bank, pin, &reg, &shift, &mask);

	val <<= shift;
	val &= mask;

	writel((readl(reg) & ~mask) | val, reg);
}

void gpio_set_data(const feldev_handle *dev, uint32_t bank, uint32_t pin, uint32_t val)
{
	uint32_t reg, shift, mask;

	sunxi_data_reg(dev, bank, pin, &reg, &shift, &mask);

	val <<= shift;
	val &= mask;

	writel((readl(reg) & ~mask) | val, reg);
}

void gpio_set_dlevel(const feldev_handle *dev, uint32_t bank, uint32_t pin, uint32_t val)
{
	uint32_t reg, shift, mask;

	sunxi_dlevel_reg(dev, bank, pin, &reg, &shift, &mask);

	val <<= shift;
	val &= mask;

	writel((readl(reg) & ~mask) | val, reg);
}

void gpio_set_pull(const feldev_handle *dev, uint32_t bank, uint32_t pin, uint32_t val)
{
	uint32_t reg, shift, mask;

	sunxi_pull_reg(dev, bank, pin, &reg, &shift, &mask);

	val <<= shift;
	val &= mask;

	writel((readl(reg) & ~mask) | val, reg);
}