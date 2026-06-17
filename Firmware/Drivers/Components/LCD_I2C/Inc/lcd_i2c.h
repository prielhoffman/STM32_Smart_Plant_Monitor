#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#include <stdint.h>
#include "stm32g071xx_i2c_driver.h"

/*
 * Public LCD API.
 * These functions are used by the application layer.
 */
void LCD_Init(I2C_Handle_t *pI2CHandle, uint8_t lcd_addr);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);

#endif
