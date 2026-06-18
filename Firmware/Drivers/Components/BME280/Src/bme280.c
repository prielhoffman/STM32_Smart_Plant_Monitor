#include "bme280.h"
#include <stddef.h>

#define BME280_REG_CHIP_ID      0xD0U

#define BME280_REG_CALIB_00     0x88U
#define BME280_REG_HUM_CALIB_00 0xA1U
#define BME280_REG_HUM_CALIB_01 0xE1U

#define BME280_CALIB_00_LEN     26U
#define BME280_HUM_CALIB_LEN    7U

static I2C_Handle_t *g_bme280_i2c_handle = NULL;
static uint8_t g_bme280_i2c_addr  = 0U;

static uint8_t BME280_ReadRegisters(uint8_t start_reg_addr, uint8_t *data, uint8_t length) {
    if ((g_bme280_i2c_handle == NULL) || (data == NULL) || (length == 0U)) {
        return 0U;
    }

    /*
     * BME280 multi-byte register read over I2C:
     * 1. Send the first register address.
     * 2. Read a sequence of bytes starting from that register.
     */
    I2C_MasterSendData(g_bme280_i2c_handle, &start_reg_addr, 1U, g_bme280_i2c_addr);
    I2C_MasterReceiveData(g_bme280_i2c_handle,data,length,g_bme280_i2c_addr);

    return 1U;
}

static uint8_t BME280_ReadRegister(uint8_t reg_addr, uint8_t *data){
	return BME280_ReadRegisters(reg_addr, data, 1U);
}

void BME280_Init(I2C_Handle_t *pI2CHandle, uint8_t device_addr){
	g_bme280_i2c_handle = pI2CHandle;
	g_bme280_i2c_addr = device_addr;
}

uint8_t BME280_ReadChipID(uint8_t *chip_id){
	return BME280_ReadRegister(BME280_REG_CHIP_ID, chip_id);
}

uint8_t BME280_ReadCalibrationData(BME280_CalibData_t *calib_data){
    uint8_t calib_00[BME280_CALIB_00_LEN];
    uint8_t hum_h1 = 0U;
    uint8_t hum_calib[BME280_HUM_CALIB_LEN];

    if (calib_data == NULL){
        return 0U;
    }

    if (!BME280_ReadRegisters(BME280_REG_CALIB_00, calib_00, BME280_CALIB_00_LEN)){
        return 0U;
    }

    if (!BME280_ReadRegister(BME280_REG_HUM_CALIB_00, &hum_h1)){
        return 0U;
    }

    if (!BME280_ReadRegisters(BME280_REG_HUM_CALIB_01, hum_calib, BME280_HUM_CALIB_LEN)){
        return 0U;
    }

    /*
     * Temperature calibration coefficients.
     * Stored little-endian in the BME280 register map.
     */
    calib_data->dig_T1 = (uint16_t)((calib_00[1] << 8) | calib_00[0]);
    calib_data->dig_T2 = (int16_t)((calib_00[3] << 8) | calib_00[2]);
    calib_data->dig_T3 = (int16_t)((calib_00[5] << 8) | calib_00[4]);

    /*
     * Pressure calibration coefficients.
     */
    calib_data->dig_P1 = (uint16_t)((calib_00[7] << 8) | calib_00[6]);
    calib_data->dig_P2 = (int16_t)((calib_00[9] << 8) | calib_00[8]);
    calib_data->dig_P3 = (int16_t)((calib_00[11] << 8) | calib_00[10]);
    calib_data->dig_P4 = (int16_t)((calib_00[13] << 8) | calib_00[12]);
    calib_data->dig_P5 = (int16_t)((calib_00[15] << 8) | calib_00[14]);
    calib_data->dig_P6 = (int16_t)((calib_00[17] << 8) | calib_00[16]);
    calib_data->dig_P7 = (int16_t)((calib_00[19] << 8) | calib_00[18]);
    calib_data->dig_P8 = (int16_t)((calib_00[21] << 8) | calib_00[20]);
    calib_data->dig_P9 = (int16_t)((calib_00[23] << 8) | calib_00[22]);

    /*
     * Humidity calibration coefficients.
     * H4 and H5 are packed across byte boundaries.
     */
    calib_data->dig_H1 = hum_h1;
    calib_data->dig_H2 = (int16_t)((hum_calib[1] << 8) | hum_calib[0]);
    calib_data->dig_H3 = hum_calib[2];

    calib_data->dig_H4 = (int16_t)(((int16_t)((int8_t)hum_calib[3]) << 4) | (hum_calib[4] & 0x0FU));

    calib_data->dig_H5 = (int16_t)(((int16_t)((int8_t)hum_calib[5]) << 4) | (hum_calib[4] >> 4));

    calib_data->dig_H6 = (int8_t)hum_calib[6];

    return 1U;
}
