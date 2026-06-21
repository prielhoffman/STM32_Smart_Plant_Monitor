#include "ds3231.h"

#include <stddef.h>

static I2C_Handle_t *g_ds3231_i2c_handle = NULL;
static uint8_t g_ds3231_i2c_addr = 0U;

/*
 * Convert a normal decimal value to BCD
 * Example:
 * decimal 45 -> BCD 0x45
 */
static uint8_t DS3231_DecimalToBCD(uint8_t decimal_value){
    return (uint8_t)(((decimal_value / 10U) << 4U) | (decimal_value % 10U));
}

/*
 * Convert a BCD value to normal decimal
 * Example:
 * BCD 0x45 -> decimal 45
 */
static uint8_t DS3231_BCDToDecimal(uint8_t bcd_value){
    return (uint8_t)(((bcd_value >> 4U) * 10U) + (bcd_value & 0x0FU));
}

/*
 * Read multiple consecutive registers from the DS3231
 * First we send the start register address
 * Then we read 'len' bytes starting from that register
 */
static uint8_t DS3231_ReadRegisters(uint8_t start_reg_addr, uint8_t *rx_buffer, uint8_t len){
    if ((g_ds3231_i2c_handle == NULL) || (rx_buffer == NULL) || (len == 0U)){
        return 0U;
    }

    I2C_MasterSendData(g_ds3231_i2c_handle, &start_reg_addr, 1U, g_ds3231_i2c_addr);
    I2C_MasterReceiveData(g_ds3231_i2c_handle, rx_buffer, len, g_ds3231_i2c_addr);

    return 1U;
}

/*
 * Write multiple consecutive registers to the DS3231
 * The transmit buffer format is:
 * byte 0: start register address
 * byte 1..n: data bytes
 */
static uint8_t DS3231_WriteRegisters(uint8_t start_reg_addr, const uint8_t *tx_data, uint8_t len){
    uint8_t tx_buffer[8];

    if ((g_ds3231_i2c_handle == NULL) || (tx_data == NULL) || (len == 0U)){
        return 0U;
    }

    /*
     * For now we only need to write 7 time/date registers
     * tx_buffer has 8 bytes:
     * 1 byte register address + 7 bytes data
     */
    if (len > 7U){
        return 0U;
    }

    tx_buffer[0] = start_reg_addr;

    for (uint8_t i = 0U; i < len; i++){
        tx_buffer[i + 1U] = tx_data[i];
    }

    I2C_MasterSendData(g_ds3231_i2c_handle, tx_buffer, (uint32_t)(len + 1U), g_ds3231_i2c_addr);

    return 1U;
}

void DS3231_Init(I2C_Handle_t *pI2CHandle, uint8_t device_addr){
    g_ds3231_i2c_handle = pI2CHandle;
    g_ds3231_i2c_addr = device_addr;
}

uint8_t DS3231_GetDateTime(DS3231_DateTime_t *date_time){
    uint8_t rx_buffer[7];

    if (date_time == NULL){
        return 0U;
    }

    if (!DS3231_ReadRegisters(DS3231_REG_SECONDS, rx_buffer, 7U)){
        return 0U;
    }

    /*
     * DS3231 stores time/date values in BCD format
     * We convert each value to normal decimal before returning it
     * seconds register:
     * bit 7 can be ignored/masked out
     * hours register:
     * this code assumes 24-hour mode
     */
    date_time->seconds = DS3231_BCDToDecimal(rx_buffer[0] & 0x7FU);
    date_time->minutes = DS3231_BCDToDecimal(rx_buffer[1] & 0x7FU);
    date_time->hours   = DS3231_BCDToDecimal(rx_buffer[2] & 0x3FU);

    date_time->day     = DS3231_BCDToDecimal(rx_buffer[3] & 0x07U);
    date_time->date    = DS3231_BCDToDecimal(rx_buffer[4] & 0x3FU);
    date_time->month   = DS3231_BCDToDecimal(rx_buffer[5] & 0x1FU);
    date_time->year    = DS3231_BCDToDecimal(rx_buffer[6]);

    return 1U;
}

uint8_t DS3231_SetDateTime(const DS3231_DateTime_t *date_time){
    uint8_t tx_buffer[7];

    if (date_time == NULL){
        return 0U;
    }

    /*
     * Convert normal decimal values to BCD before writing them to the DS3231
     * We use 24-hour mode, so the hours register is written as a normal BCD value
     */
    tx_buffer[0] = DS3231_DecimalToBCD(date_time->seconds);
    tx_buffer[1] = DS3231_DecimalToBCD(date_time->minutes);
    tx_buffer[2] = DS3231_DecimalToBCD(date_time->hours);

    tx_buffer[3] = DS3231_DecimalToBCD(date_time->day);
    tx_buffer[4] = DS3231_DecimalToBCD(date_time->date);
    tx_buffer[5] = DS3231_DecimalToBCD(date_time->month);
    tx_buffer[6] = DS3231_DecimalToBCD(date_time->year);

    if (!DS3231_WriteRegisters(DS3231_REG_SECONDS, tx_buffer, 7U)){
        return 0U;
    }

    return 1U;
}
