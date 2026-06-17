#include "lcd_i2c.h"
#include <stddef.h>

/*
 * This driver controls a 16x2 LCD through an I2C backpack.
 * The low-level I2C communication is handled by the STM32G071 I2C driver.
 */

/*
 * Common PCF8574 LCD backpack bit mapping:
 *
 * P0 -> RS
 * P1 -> RW
 * P2 -> EN
 * P3 -> Backlight
 * P4 -> D4
 * P5 -> D5
 * P6 -> D6
 * P7 -> D7
 */
#define LCD_RS_BIT          0U
#define LCD_RW_BIT          1U
#define LCD_EN_BIT          2U
#define LCD_BACKLIGHT_BIT   3U

#define LCD_BACKLIGHT_ON    (1U << LCD_BACKLIGHT_BIT)

#define LCD_CMD_CLEAR_DISPLAY       0x01U
#define LCD_CMD_RETURN_HOME         0x02U
#define LCD_CMD_ENTRY_MODE_SET      0x06U
#define LCD_CMD_DISPLAY_ON          0x0CU
#define LCD_CMD_FUNCTION_SET_4BIT   0x28U
#define LCD_CMD_SET_DDRAM_ADDR      0x80U

static I2C_Handle_t *g_lcd_i2c_handle = NULL;
static uint8_t g_lcd_i2c_addr = 0U;

static void LCD_Delay(void){
    for (volatile uint32_t i = 0; i < 50000U; i++);
}

static void LCD_I2C_WriteByte(uint8_t data){
	if (g_lcd_i2c_handle == NULL){
		return;
	}
	I2C_MasterSendData(g_lcd_i2c_handle, &data, 1U, g_lcd_i2c_addr);
}

static void LCD_SendNibble(uint8_t nibble, uint8_t rs){
	uint8_t data = 0U;

    /*
     * The LCD data lines D4-D7 are connected to PCF8574 bits P4-P7.
     * Therefore, the 4-bit nibble is shifted left by 4 positions.
     */
	data = (uint8_t)((nibble & 0X0FU) << 4);

	if (rs != 0U){
		data |= (1U << LCD_RS_BIT);
	}

	data |= LCD_BACKLIGHT_ON;

    /*
     * Generate an Enable pulse:
     * 1. Send data with EN = 1
     * 2. Send the same data with EN = 0
     * The falling edge tells the LCD to latch the nibble.
     */

	LCD_I2C_WriteByte(data | (1U << LCD_EN_BIT));
	LCD_I2C_WriteByte(data & ~(1U << LCD_EN_BIT));
}

static void LCD_SendByte(uint8_t value, uint8_t rs){
	uint8_t upper = (uint8_t)((value >> 4) & 0x0FU);
	uint8_t lower = (uint8_t)((value) & 0x0FU);

	LCD_SendNibble(upper, rs);
	LCD_SendNibble(lower, rs);
}

static void LCD_SendCommand(uint8_t command){
	LCD_SendByte(command, 0U);
}

static void LCD_SendData(uint8_t data){
	LCD_SendByte(data, 1U);
}

void LCD_Init(I2C_Handle_t *pI2CHandle, uint8_t lcd_addr){
    g_lcd_i2c_handle = pI2CHandle;
    g_lcd_i2c_addr = lcd_addr;

    /* Wait after power-up before sending the LCD initialization sequence */
    LCD_Delay();
    LCD_Delay();

    /* Initialize the LCD into 4-bit mode */
    LCD_SendNibble(0x03U, 0U);
    LCD_Delay();

    LCD_SendNibble(0x03U, 0U);
    LCD_Delay();

    LCD_SendNibble(0x03U, 0U);
    LCD_Delay();

    LCD_SendNibble(0x02U, 0U);
    LCD_Delay();

    /*
     * Configure the LCD:
     * - 4-bit mode
     * - 2 lines
     * - 5x8 font
     * - display on
     * - cursor off
     * - auto-increment cursor
     */
    LCD_SendCommand(LCD_CMD_FUNCTION_SET_4BIT);
    LCD_Delay();

    LCD_SendCommand(LCD_CMD_DISPLAY_ON);
    LCD_Delay();

    LCD_SendCommand(LCD_CMD_CLEAR_DISPLAY);
    LCD_Delay();
    LCD_Delay();

    LCD_SendCommand(LCD_CMD_ENTRY_MODE_SET);
    LCD_Delay();
}

void LCD_Clear(void){
	LCD_SendCommand(LCD_CMD_CLEAR_DISPLAY);
	LCD_Delay();
	LCD_Delay();
}

void LCD_SetCursor(uint8_t row, uint8_t col){
	uint8_t row_offset = 0U;

	if (row == 0U){
		row_offset = 0x00U;
	}
	else{
		row_offset = 0x40U;
	}
	LCD_SendCommand((uint8_t)LCD_CMD_SET_DDRAM_ADDR | (row_offset + col));
	LCD_Delay();
}

void LCD_Print(const char *str){
	if (str == NULL){
		return;
	}

	while (*str != '\0'){
		LCD_SendData((uint8_t)*str);
		str++;
	}
}
