/*
 * (C) Copyright 2024 Iscle <albertiscle9@gmail.com>
 */
#include "fel-i2c.h"
#include "fel-gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define debug(fmt, ...) printf("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

uint32_t fel_readl(feldev_handle *dev, uint32_t addr);
void fel_writel(feldev_handle *dev, uint32_t addr, uint32_t val);
#define readl(addr)		fel_readl(dev, (addr))
#define writel(val, addr)	fel_writel(dev, (addr), (val))

#define SUN50I_I2C_DATA	(0x07081400 + 0x08)
#define SUN50I_I2C_CNTR	(0x07081400 + 0x0c)
#define SUN50I_I2C_STAT	(0x07081400 + 0x10)
#define SUN50I_I2C_CCR	(0x07081400 + 0x14)
#define SUN50I_I2C_SRST	(0x07081400 + 0x18)
#define SUN50I_I2C_EFR	(0x07081400 + 0x1c)
#define SUN50I_I2C_LCR	(0x07081400 + 0x20)

static int i2c_send_start(feldev_handle *dev)
{
	uint32_t reg_val;
	int cnt;

	writel(0, SUN50I_I2C_EFR);
	writel(1, SUN50I_I2C_SRST);

	reg_val = readl(SUN50I_I2C_CNTR);
	reg_val |= (1 << 5); // M_STA
	writel(reg_val, SUN50I_I2C_CNTR);
	
	cnt = 0xff;
	while (!(readl(SUN50I_I2C_CNTR) & (1 << 3))) {
		if (cnt-- == 0) {
			debug("I2C timeout\n");
			return -1;
		}
	}

	reg_val = readl(SUN50I_I2C_STAT);
	if (reg_val != 0x08) {
		debug("I2C start failed (0x%02x)\n", reg_val);
		return -1;
	}
	
	return 0;
}

static int i2c_send_slave_addr(feldev_handle *dev, uint8_t addr, int rw)
{
	uint32_t reg_val;
	int cnt;

	writel((addr << 1) | (rw ? 1 : 0), SUN50I_I2C_DATA);

	/* Clear INT_FLAG */
	reg_val = readl(SUN50I_I2C_CNTR);
	reg_val |= (1 << 3);
	writel(reg_val, SUN50I_I2C_CNTR);

	cnt = 0xff;
	while (!(readl(SUN50I_I2C_CNTR) & (1 << 3))) {
		if (cnt-- == 0) {
			debug("I2C timeout\n");
			return -1;
		}
	}

	reg_val = readl(SUN50I_I2C_STAT);
	if (reg_val != (rw ? 0x40 : 0x18)) {
		debug("I2C send slave addr failed (0x%02x)\n", reg_val);
		return -1;
	}

	return 0;
}

static int i2c_send_byte_addr(feldev_handle *dev, uint8_t addr)
{
	uint32_t reg_val;
	int cnt;

	writel(addr, SUN50I_I2C_DATA);

	/* Clear INT_FLAG */
	reg_val = readl(SUN50I_I2C_CNTR);
	reg_val |= (1 << 3);
	writel(reg_val, SUN50I_I2C_CNTR);

	cnt = 0xff;
	while (!(readl(SUN50I_I2C_CNTR) & (1 << 3))) {
		if (cnt-- == 0) {
			debug("I2C timeout\n");
			return -1;
		}
	}

	reg_val = readl(SUN50I_I2C_STAT);
	if (reg_val != 0x28) {
		debug("I2C send byte addr failed (0x%02x)\n", reg_val);
		return -1;
	}
	
	return 0;
}

static int i2c_send_restart(feldev_handle *dev)
{
	uint32_t reg_val;
	int cnt;

	reg_val = readl(SUN50I_I2C_CNTR);
	reg_val |= (1 << 5); // M_STA
	writel(reg_val, SUN50I_I2C_CNTR);

	cnt = 0xff;
	while (!(readl(SUN50I_I2C_CNTR) & (1 << 3))) {
		if (cnt-- == 0) {
			debug("I2C timeout\n");
			return -1;
		}
	}

	reg_val = readl(SUN50I_I2C_STAT);
	if (reg_val != 0x10) {
		debug("I2C send restart failed (0x%02x)\n", reg_val);
		return -1;
	}

	return 0;
}

static int i2c_get_data(feldev_handle *dev, uint8_t *buffer, size_t len)
{
	uint32_t reg_val;
	int cnt;

	if (len == 1) {
		/* Clear INT_FLAG */
		reg_val = readl(SUN50I_I2C_CNTR);
		reg_val |= (1 << 3);
		writel(reg_val, SUN50I_I2C_CNTR);

		cnt = 0xff;
		while (!(readl(SUN50I_I2C_CNTR) & (1 << 3))) {
			if (cnt-- == 0) {
				debug("I2C timeout\n");
				return -1;
			}
		}

		*buffer = readl(SUN50I_I2C_DATA);

		reg_val = readl(SUN50I_I2C_STAT);
		if (reg_val != 0x58) {
			debug("I2C get data failed (0x%02x)\n", reg_val);
			return -1;
		}
	} else  {
		printf("I2C read with len > 1 not implemented yet!\n");
		return -1;
	}

	return 0;
}

static int i2c_send_data(feldev_handle *dev, uint8_t *buffer, size_t len)
{
	uint32_t reg_val;
	int cnt;

	while (len--) {
		writel(*buffer++, SUN50I_I2C_DATA);

		/* Clear INT_FLAG */
		reg_val = readl(SUN50I_I2C_CNTR);
		reg_val |= (1 << 3);
		writel(reg_val, SUN50I_I2C_CNTR);

		cnt = 0xff;
		while (!(readl(SUN50I_I2C_CNTR) & (1 << 3))) {
			if (cnt-- == 0) {
				debug("I2C timeout\n");
				return -1;
			}
		}

		reg_val = readl(SUN50I_I2C_STAT);
		if (reg_val != 0x28) {
			debug("I2C send data failed (0x%02x)\n", reg_val);
			return -1;
		}
	}

	return 0;
}

static int i2c_stop(feldev_handle *dev)
{
	uint32_t reg_val;
	int cnt;

	reg_val = readl(SUN50I_I2C_CNTR);
	reg_val |= (1 << 4); // M_STP
	writel(reg_val, SUN50I_I2C_CNTR);

	/* Clear INT_FLAG */
	reg_val = readl(SUN50I_I2C_CNTR);
	reg_val |= (1 << 3);
	writel(reg_val, SUN50I_I2C_CNTR);

	cnt = 0xff;
	while (readl(SUN50I_I2C_CNTR) & (1 << 4)) {
		if (cnt-- == 0) {
			debug("I2C timeout\n");
			return -1;
		}
	}

	cnt = 0xff;
	while (readl(SUN50I_I2C_STAT) != 0xf8) {
		if (cnt-- == 0) {
			debug("I2C timeout\n");
			return -1;
		}
	}

	return 0;
}

void i2c_init(feldev_handle *dev)
{
	uint32_t reg_val;
	int cnt;

	/* R_PRCM */
	reg_val = readl(0x07010254);
	reg_val |= 2;
	writel(reg_val, 0x07010254);

	/* SYS_CFG */
	reg_val = readl(0x03000160);
	reg_val |= 2;
	writel(reg_val, 0x03000160);

	reg_val = readl(0x03000160);
	reg_val &= ~1;
	writel(reg_val, 0x03000160);

	reg_val = readl(0x03000160);
	reg_val |= 1;
	writel(reg_val, 0x03000160);

	/* IOMMU */
	writel(1, 0x030017bc);
	writel(1, 0x030f0040);
	writel(0x301, 0x03001500);

	/* CCU */
	// _DAT_03001000 = _DAT_03001000 & 0xf7fc00fc | 0x20002900;
	// do {
	// } while (-1 < (int)(_DAT_03001000 << 3));
	// udelay(0x14);
	// _DAT_03001000 = _DAT_03001000 & 0xdfffffff | 0x8000000;
	// _DAT_03001500 = _DAT_03001500 | 0x3000000;
	// udelay(1);
	// if (-1 < _DAT_03001020) {
	// do {
	// } while( true );
	// }
	// printf("periph0 has been enabled\n");
	// _DAT_03001510 = 0x3000002;
	// udelay(1);
	// _DAT_0300151c = 0x3000002;
	// _DAT_03001520 = 0x3000102;
	// udelay(1);
	// _DAT_0300170c = _DAT_0300170c | 0x10000;
	// udelay(0x14);
	// _DAT_0300170c = _DAT_0300170c | 1;
	// _DAT_03001540 = 0x40000000;
	// udelay(1);
	// _DAT_03001540 = _DAT_03001540 | 2;
	// udelay(1);
	// _DAT_03001540 = _DAT_03001540 | 0x1000000;
	// udelay(1);
	// _DAT_03001540 = _DAT_03001540 | 0x80000000;
	// udelay(1);
	// local_38[0] = (uint *)&DAT_03001020;
	// local_38[1] = (uint *)&DAT_03001028;
	// local_38[2] = (uint *)0x3001030;
	// local_38[3] = (uint *)0x3001040;
	// local_38[4] = (uint *)0x3001048;
	// local_38[5] = (uint *)0x3001050;
	// local_38[6] = (uint *)0x3001068;
	// local_38[7] = (uint *)0x3001058;
	// local_38[8] = (uint *)0x3001060;
	// local_38[9] = (uint *)0x3001078;
	// ppuVar3 = &puStack_3c;
	// do {
	// ppuVar3 = ppuVar3 + 1;
	// if (-1 < (int)**ppuVar3) {
	// puVar2 = *ppuVar3;
	// *puVar2 = **ppuVar3;
	// puVar1 = *ppuVar3;
	// *puVar1 = *puVar2 | 0x80000000;
	// **ppuVar3 = *puVar1 | 0x20000000;
	// do {
	// } while (-1 < (int)(**ppuVar3 << 3));
	// udelay(0x14);
	// **ppuVar3 = **ppuVar3 & 0xdfffffff;
	// }
	// } while (ppuVar3 != local_38 + 9);

	/* set_rpio_power_mode start */

	if (readl(0x07022348) & 1) {
		printf("PL gpio voltage : 1.8V \n");
		writel(1, 0x07022340);
	} else {
		printf("PL gpio voltage : 3.3V \n");
	}

	/* set_rpio_power_mode end */
  	
	/* set_cpus_i2c_clock start */

 	/* R_GPIO */
	reg_val = readl(0x07022000);
	reg_val &= ~0xff;
	reg_val |= 0x22;
	writel(reg_val, 0x07022000);

	reg_val = readl(0x0702201c);
	reg_val &= ~0xf;
	reg_val |= 5;
	writel(reg_val, 0x0702201c);

	reg_val = readl(0x07022014);
	reg_val &= ~0xf;
	writel(reg_val, 0x07022014);

	/* R_PRCM */
	reg_val = readl(0x0701019c);
	reg_val |= 0x10001;
	reg_val &= ~0x10000;
	reg_val |= 0x10000;
	writel(reg_val, 0x0701019c);

	reg_val = readl(0x0701019c);
	reg_val |= 1;
	writel(reg_val, 0x0701019c);

	/* set_cpus_i2c_clock end */

	/* i2c_set_clock */

	// Reset I2C controller
	writel(1, SUN50I_I2C_SRST);
	cnt = 0xff;
	while (readl(SUN50I_I2C_SRST) & 1) {
		if (cnt-- == 0) {
			debug("I2C timeout\n");
			return;
		}
	}

	if ((readl(SUN50I_I2C_LCR) & 0x30) != 0x30) {
		debug("I2C bus not in idle state\n");
		/* toggle I2C SCL and SDA until bus idle */
		writel(0x05, SUN50I_I2C_LCR);
		int i = 10;
		while ((i > 0) && ((readl(SUN50I_I2C_LCR) & 0x02) != 2)) {
			/*control scl and sda output high level*/
			reg_val = readl(SUN50I_I2C_LCR);
			reg_val |= 0x08;
			writel(reg_val, SUN50I_I2C_LCR);
			reg_val = readl(SUN50I_I2C_LCR);
			reg_val |= 0x02;
			writel(reg_val, SUN50I_I2C_LCR);
			/*control scl and sda output low level*/
			reg_val = readl(SUN50I_I2C_LCR);
			reg_val &= ~0x08;
			writel(reg_val, SUN50I_I2C_LCR);
			reg_val = readl(SUN50I_I2C_LCR);
			reg_val &= ~0x02;
			writel(reg_val, SUN50I_I2C_LCR);
			i--;
		}
		writel(0, SUN50I_I2C_LCR);
	}

	// Set 400 kHz clock
	writel(((2400 / 400) - 1) << 3, SUN50I_I2C_CCR);
	writel(1 << 6, SUN50I_I2C_CNTR);
	writel(0, SUN50I_I2C_EFR);

	/* i2c_set_clock end */
}

int i2c_read(feldev_handle *dev, uint8_t addr, uint8_t reg, uint8_t *buffer, size_t len)
{
	int ret;

	ret  = i2c_send_start(dev);
	if (ret)
		goto err;

	ret = i2c_send_slave_addr(dev, addr, 0);
	if (ret)
		goto err;

	ret = i2c_send_byte_addr(dev, reg);
	if (ret)
		goto err;

	ret = i2c_send_restart(dev);
	if (ret)
		goto err;

	ret = i2c_send_slave_addr(dev, addr, 1);
	if (ret)
		goto err;

	ret = i2c_get_data(dev, buffer, len);
	if (ret)
		goto err;

err:
	i2c_stop(dev);

	return ret;
}

int i2c_write(feldev_handle *dev, uint8_t addr, uint8_t reg, uint8_t *buffer, size_t len)
{
	int ret;
	uint8_t *ptr = buffer;

	ret  = i2c_send_start(dev);
	if (ret)
		goto err;

	ret = i2c_send_slave_addr(dev, addr, 0);
	if (ret)
		goto err;

	ret = i2c_send_byte_addr(dev, reg);
	if (ret)
		goto err;

	ret = i2c_send_data(dev, ptr, len);

err:
	i2c_stop(dev);

	return ret;
}
