#ifndef BME280_H_
#define BME280_H_

#include "stm32g071xx_i2c_driver.h"
#include <stdint.h>

#define BME280_CHIP_ID_VALUE    0x60U

typedef struct{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} BME280_CalibData_t;

typedef struct{
    int32_t raw_temperature;
    int32_t raw_pressure;
    int32_t raw_humidity;
} BME280_RawData_t;

void BME280_Init(I2C_Handle_t *pI2CHandle, uint8_t device_addr);
uint8_t BME280_ReadChipID(uint8_t *chip_id);
uint8_t BME280_ReadCalibrationData(BME280_CalibData_t *calib_data);
uint8_t BME280_ConfigureMeasurement(void);
uint8_t BME280_ReadRawData(BME280_RawData_t *raw_data);

#endif
