#include "bme280.h"
#include <stddef.h>

#define BME280_REG_CHIP_ID      0xD0U

static I2C_Handle_t *g_bme280_i2c_handle = NULL;
static uint8_t g_bme280_i2c_addr  = 0U;

static uint8_t BME280_ReadRegister(uint8_t reg_addr, uint8_t *data){
	if ((g_bme280_i2c_handle == NULL) || (data == NULL)){
		return 0U;
	}

    /*
     * BME280 register read over I2C:
     * 1. Send the register address.
     * 2. Read one byte from that register.
     */
	I2C_MasterSendData(g_bme280_i2c_handle, &reg_addr, 1U, g_bme280_i2c_addr);
	I2C_MasterReceiveData(g_bme280_i2c_handle, data, 1U, g_bme280_i2c_addr);
	return 1U;
}

void BME280_Init(I2C_Handle_t *pI2CHandle, uint8_t device_addr){
	g_bme280_i2c_handle = pI2CHandle;
	g_bme280_i2c_addr = device_addr;
}

uint8_t BME280_ReadChipID(uint8_t *chip_id){
	return BME280_ReadRegister(BME280_REG_CHIP_ID, chip_id);
}
