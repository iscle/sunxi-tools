static int axp2202_init(feldev_handle *dev)
{
	uint8_t buf;
	int ret;

	ret = i2c_read(dev, 0x34, 0x0e, &buf, 1); /* CHIP_ID_EXT */
	if (ret || buf != 0x02)
		return -1;

	printf("Found AXP2202 @ 0x34\n");

	/* Limit current to 2A */
	buf = 0x26;
	i2c_write(dev, 0x34, 0x17, &buf, 1); /* VBUS_CUR_SET */

	/* Enable ADC channel 0 */
	i2c_read(dev, 0x34, 0xc0, &buf, 1); /* ADC_CH0 */
	buf |= 0x33;
	i2c_write(dev, 0x34, 0xc0, &buf, 1);

	/* Set VSYS min */
	i2c_read(dev, 0x34, 0x15, &buf, 1); /* VSYS_MIN */
	buf = 0x06;
	i2c_write(dev, 0x34, 0x15, &buf, 1);

	/* Disable DCDC1 UVP */
	i2c_read(dev, 0x34, 0x23, &buf, 1); /* DCDC_PWEOFF_EN */
	buf &= ~(1 << 0);
	i2c_write(dev, 0x34, 0x23, &buf, 1);

	i2c_read(dev, 0x34, 0x0f, &buf, 1); /* CHIP_VER_EXT */
	if (buf) {
		i2c_read(dev, 0x34, 0x19, &buf, 1); /* MODULE_EN */
		buf |= 0x10;
		i2c_write(dev, 0x34, 0x19, &buf, 1);
	} else {
		i2c_read(dev, 0x34, 0x19, &buf, 1); /* MODULE_EN */
		buf &= 0xEF;
		i2c_write(dev, 0x34, 0x19, &buf, 1);
	}

	/* Set cldo1 voltage */\
	i2c_read(dev, 0x34, 0x9b, &buf, 1); /* AXP2202_CLDO1_CFG */
	debug("CLDO1_CFG: 0x%02x\n", buf);

	/* Enable cldo1 */
	i2c_read(dev, 0x34, 0x91, &buf, 1); /* AXP2202_LDO_EN_CFG1 */
	debug("LDO_EN_CFG1: 0x%02x\n", buf);

	/* Set cldo3 voltage */
	i2c_read(dev, 0x34, 0x9d, &buf, 1); /* AXP2202_CLDO3_CFG */
	debug("CLDO3_CFG: 0x%02x\n", buf);
	// buf &= ~0x1f;
	// buf |= 0x0a;
	// i2c_write(dev, 0x34, 0x9d, &buf, 1);

	/* Enable cldo3 */
	i2c_read(dev, 0x34, 0x91, &buf, 1); /* AXP2202_LDO_EN_CFG1 */
	debug("LDO_EN_CFG1: 0x%02x\n", buf);
	// buf |= (1 << 2);
	// i2c_write(dev, 0x34, 0x91, &buf, 1);


	return 0;
}

static int axp_init(feldev_handle *dev)
{
	int ret;
	
	ret = axp2202_init(dev);
	if (ret == 0)
		return 0;

	printf("No AXP found!\n");
	return -1;

}
