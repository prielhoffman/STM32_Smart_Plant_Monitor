#include "bme280.h"
#include <stddef.h>

#define BME280_REG_CHIP_ID      0xD0U

#define BME280_REG_CALIB_00     0x88U
#define BME280_REG_HUM_CALIB_00 0xA1U
#define BME280_REG_HUM_CALIB_01 0xE1U

#define BME280_CALIB_00_LEN     26U
#define BME280_HUM_CALIB_LEN    7U

#define BME280_REG_CTRL_HUM     0xF2U
#define BME280_REG_CTRL_MEAS    0xF4U
#define BME280_REG_CONFIG       0xF5U
#define BME280_REG_PRESS_MSB    0xF7U

#define BME280_RAW_DATA_LEN     8U

static I2C_Handle_t *g_bme280_i2c_handle = NULL;
static uint8_t g_bme280_i2c_addr  = 0U;
static BME280_CalibData_t g_bme280_calib_data;
static int32_t g_bme280_t_fine;

static BME280_CalibData_t g_bme280_calib_data;
static int32_t g_bme280_t_fine = 0;
static uint8_t g_bme280_calib_loaded = 0U;

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

static uint8_t BME280_WriteRegister(uint8_t reg_addr, uint8_t data){
	uint8_t tx_buffer[2];

	if (g_bme280_i2c_handle == NULL){
		return 0U;
	}

	tx_buffer[0] = reg_addr;
	tx_buffer[1] = data;

	I2C_MasterSendData(g_bme280_i2c_handle, tx_buffer, 2U, g_bme280_i2c_addr);
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

    g_bme280_calib_data = *calib_data;
    g_bme280_calib_loaded = 1U;

    return 1U;
}

uint8_t BME280_ConfigureMeasurement(void){
    /*
     * Humidity oversampling x1.
     * This must be written before ctrl_meas according to the BME280 flow.
     */
    if (!BME280_WriteRegister(BME280_REG_CTRL_HUM, 0x01U)){
        return 0U;
    }

    /*
     * Temperature oversampling x1, pressure oversampling x1, normal mode.
     * osrs_t = x1  -> bits 7:5 = 001
     * osrs_p = x1  -> bits 4:2 = 001
     * mode   = normal -> bits 1:0 = 11
     * 001 001 11 = 0x27
     */
    if (!BME280_WriteRegister(BME280_REG_CTRL_MEAS, 0x27U)){
        return 0U;
    }

    /*
     * Basic config:
     * standby time 1000 ms, filter off.
     * This is fine for a slow plant monitor.
     */
    if (!BME280_WriteRegister(BME280_REG_CONFIG, 0xA0U)){
        return 0U;
    }

    return 1U;
}

uint8_t BME280_ReadRawData(BME280_RawData_t *raw_data){
    uint8_t data[BME280_RAW_DATA_LEN];

    if (raw_data == NULL){
        return 0U;
    }

    if (!BME280_ReadRegisters(BME280_REG_PRESS_MSB, data, BME280_RAW_DATA_LEN)){
        return 0U;
    }

    /*
     * Raw pressure and raw temperature are 20-bit values.
     * Raw humidity is a 16-bit value.
     */
    raw_data->raw_pressure = (int32_t)(((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4)  | ((uint32_t)data[2] >> 4));
    raw_data->raw_temperature = (int32_t)(((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4)  | ((uint32_t)data[5] >> 4));
    raw_data->raw_humidity = (int32_t)(((uint32_t)data[6] << 8) | ((uint32_t)data[7]));

    return 1U;
}

static int32_t BME280_CompensateTemperature(int32_t raw_temperature){
    int32_t var1;
    int32_t var2;
    int32_t temperature;

    /*
     * Temperature compensation formula sourced from the BME280 Datasheet.
     * This uses 32-bit integer arithmetic to return temperature in Celsius x100.
     * Example: 2534 represents 25.34°C.
     */
    var1 = ((((raw_temperature >> 3) - ((int32_t)g_bme280_calib_data.dig_T1 << 1))) * ((int32_t)g_bme280_calib_data.dig_T2)) >> 11;
    var2 = (((((raw_temperature >> 4) - ((int32_t)g_bme280_calib_data.dig_T1)) * ((raw_temperature >> 4) - ((int32_t)g_bme280_calib_data.dig_T1))) >> 12) * ((int32_t)g_bme280_calib_data.dig_T3)) >> 14;
    g_bme280_t_fine = var1 + var2;
    temperature = (g_bme280_t_fine * 5 + 128) >> 8;

    return temperature;
}

uint8_t BME280_ReadTemperature(BME280_CompensatedData_t *comp_data){
	BME280_RawData_t raw_data;

	if (comp_data == NULL){
		return 0U;
	}
	if (g_bme280_calib_loaded == 0U){
		return 0U;
	}
	if (!BME280_ReadRawData(&raw_data)){
		return 0U;
	}

	comp_data->temperature_c_x100 = BME280_CompensateTemperature(raw_data.raw_temperature);
	return 1U;
}

static uint32_t BME280_CompensateHumidity(int32_t raw_humidity){
    int32_t v_x1_u32r;
    uint32_t humidity_q1024;
    uint32_t humidity_x100;

    /*
     * Humidity compensation formula sourced from the BME280 Datasheet.
     * This calculation depends on g_bme280_t_fine,
     * which is updated by BME280_CompensateTemperature().
     */
    v_x1_u32r = g_bme280_t_fine - 76800;

    v_x1_u32r = (((((raw_humidity << 14) - ((int32_t)g_bme280_calib_data.dig_H4 << 20) - ((int32_t)g_bme280_calib_data.dig_H5 * v_x1_u32r)) +
           16384) >> 15) * (((((((v_x1_u32r * (int32_t)g_bme280_calib_data.dig_H6) >> 10) * (((v_x1_u32r * (int32_t)g_bme280_calib_data.dig_H3) >> 11) +
               32768)) >> 10) +  2097152) * (int32_t)g_bme280_calib_data.dig_H2 + 8192) >> 14));

    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * (int32_t)g_bme280_calib_data.dig_H1) >> 4);

    if (v_x1_u32r < 0){
        v_x1_u32r = 0;
    }

    if (v_x1_u32r > 419430400){
        v_x1_u32r = 419430400;
    }

    /*
     * After shifting by 12, the result is humidity in %RH x1024.
     * Convert it to %RH x100 for easier printing and storage.
     */
    humidity_q1024 = (uint32_t)(v_x1_u32r >> 12);
    humidity_x100 = (humidity_q1024 * 100U) / 1024U;

    return humidity_x100;
}

uint8_t BME280_ReadTemperatureHumidity(BME280_CompensatedData_t *comp_data){
    BME280_RawData_t raw_data;

    if (comp_data == NULL){
        return 0U;
    }

    if (g_bme280_calib_loaded == 0U){
        return 0U;
    }

    if (!BME280_ReadRawData(&raw_data)){
        return 0U;
    }

    /*
     * Temperature must be compensated first because it updates t_fine.
     * Humidity compensation depends on t_fine.
     */
    comp_data->temperature_c_x100 = BME280_CompensateTemperature(raw_data.raw_temperature);

    comp_data->humidity_percent_x100 = BME280_CompensateHumidity(raw_data.raw_humidity);

    return 1U;
}
