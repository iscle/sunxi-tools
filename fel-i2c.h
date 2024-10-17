#ifndef _FEL_I2C_H_
#define _FEL_I2C_H_

#include "fel_lib.h"

void i2c_init(feldev_handle *dev);
int i2c_read(feldev_handle *dev, uint8_t addr, uint8_t reg, uint8_t *buffer, size_t len);
int i2c_write(feldev_handle *dev, uint8_t addr, uint8_t reg, uint8_t *buffer, size_t len);

#endif /* _FEL_I2C_H_ */