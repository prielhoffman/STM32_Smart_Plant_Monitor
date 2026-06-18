#ifndef BME280_H_
#define BME280_H_

#include "stm32g071xx_i2c_driver.h"
#include <stdint.h>

#define BME280_CHIP_ID_VALUE    0x60U

void BME280_Init(I2C_Handle_t *pI2CHandle, uint8_t device_addr);
uint8_t BME280_ReadChipID(uint8_t *chip_id);

#endif
