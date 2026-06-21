#ifndef DS3231_H_
#define DS3231_H_

#include <stdint.h>
#include "stm32g071xx_i2c_driver.h"

/*
 * DS3231 I2C address.
 * The DS3231 uses 0x68 as its 7-bit I2C address.
 */
#define DS3231_I2C_ADDR         0x68U

/*
 * DS3231 time/date registers.
 * The RTC stores seconds, minutes, hours, day, date, month and year
 * in consecutive registers starting from address 0x00.
 */
#define DS3231_REG_SECONDS      0x00U
#define DS3231_REG_MINUTES      0x01U
#define DS3231_REG_HOURS        0x02U
#define DS3231_REG_DAY          0x03U
#define DS3231_REG_DATE         0x04U
#define DS3231_REG_MONTH        0x05U
#define DS3231_REG_YEAR         0x06U

/*
 * Holds a full date and time value read from or written to the DS3231.
 * year is stored as two digits:
 * 26 means 2026
 * day is the day of week:
 * 1-7, according to how we choose to define it
 */
typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;

    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} DS3231_DateTime_t;

/*
 * Initialize the DS3231 component driver
 * This does not configure the RTC time
 * It only stores the I2C handle and the device address for later use
 */
void DS3231_Init(I2C_Handle_t *pI2CHandle, uint8_t device_addr);

/*
 * Read the current date and time from the DS3231
 * Returns:
 * 1 = success
 * 0 = failure
 */
uint8_t DS3231_GetDateTime(DS3231_DateTime_t *date_time);

/*
 * Set the current date and time in the DS3231
 * This should usually be called only once when setting the RTC
 * Returns:
 * 1 = success
 * 0 = failure
 */
uint8_t DS3231_SetDateTime(const DS3231_DateTime_t *date_time);

#endif
