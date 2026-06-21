#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_usart_driver.h"
#include "stm32g071xx_adc_driver.h"
#include "stm32g071xx_i2c_driver.h"
#include "stm32g071xx_spi_driver.h"
#include "lcd_i2c.h"
#include "bme280.h"
#include "ds3231.h"
#include "sd_card.h"
#include "ff.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/*
 * Sensor calibration values based on initial hardware measurements.
 *
 * Soil sensor:
 * - Wet touch: around 1100
 * - Air / dry: around 2650
 * Higher raw value means drier soil.
 *
 * Light sensor:
 * - Covered: around 1560
 * - Direct flashlight: around 3800
 * Higher raw value means more light.
 */
#define SOIL_WET_RAW            1100U
#define SOIL_DRY_RAW            2650U

#define LIGHT_DARK_RAW          1560U
#define LIGHT_BRIGHT_RAW        3800U

#define SOIL_DRY_THRESHOLD      2300U
#define LIGHT_LOW_THRESHOLD     1800U

#define RED_LED_GPIO_PORT       GPIOA
#define RED_LED_GPIO_PIN        GPIO_PIN_NO_10

#define YELLOW_LED_GPIO_PORT    GPIOB
#define YELLOW_LED_GPIO_PIN     GPIO_PIN_NO_3

#define GREEN_LED_GPIO_PORT     GPIOB
#define GREEN_LED_GPIO_PIN      GPIO_PIN_NO_5

#define LCD_I2C_ADDR            0x27U
#define BME280_I2C_ADDR         0x76U
#define LCD_PAGE_COUNT          3U

#define DS3231_I2C_ADDR         0x68U

#define SD_CS_GPIO_PORT         GPIOA
#define SD_CS_GPIO_PIN          GPIO_PIN_NO_4

#define SD_TEST_BLOCK_ADDR      100000U

typedef enum
{
    SYSTEM_STATE_INIT = 0,
    SYSTEM_STATE_READ_SENSORS,
    SYSTEM_STATE_PROCESS_DATA,
    SYSTEM_STATE_UPDATE_ALERTS,
    SYSTEM_STATE_UPDATE_DISPLAY,
    SYSTEM_STATE_PRINT_LOG,
    SYSTEM_STATE_WAIT
} SystemState_t;

typedef enum
{
    PLANT_STATUS_OK = 0,
    PLANT_STATUS_LOW_SOIL,
    PLANT_STATUS_LOW_LIGHT,
    PLANT_STATUS_LOW_SOIL_AND_LIGHT,
    PLANT_STATUS_SENSOR_ERROR
} PlantStatus_t;

/*
 * Holds the current plant monitoring data.
 * For now, soil_raw and light_raw will be simulated values.
 * Later, they will come from real ADC readings.
 */
typedef struct
{
    uint16_t soil_raw;
    uint16_t light_raw;

    uint8_t soil_percent;
    uint8_t light_percent;

    int32_t air_temperature_c_x100;
    uint32_t air_humidity_percent_x100;
    uint8_t bme280_is_available;

    DS3231_DateTime_t timestamp;
    uint8_t rtc_is_available;

    PlantStatus_t plant_status;
} PlantMonitorData_t;

static USART_Handle_t g_usart2_handle;
static I2C_Handle_t g_i2c1_handle;
static SPI_Handle_t g_spi1_handle;
static PlantMonitorData_t g_plant_data;
static SystemState_t g_current_state = SYSTEM_STATE_INIT;
static uint8_t g_lcd_page = 0U;

/* Convert system state enum to readable text for UART logs */
static const char *SystemState_ToString(SystemState_t state){
	switch(state){
	case SYSTEM_STATE_INIT:
		return "INIT";
	case SYSTEM_STATE_READ_SENSORS:
		return "READ_SENSORS";
	case SYSTEM_STATE_PROCESS_DATA:
		return "PROCESS_DATA";
	case SYSTEM_STATE_UPDATE_ALERTS:
		return "UPDATE_ALERTS";
	case SYSTEM_STATE_UPDATE_DISPLAY:
		return "UPDATE_DISPLAY";
	case SYSTEM_STATE_PRINT_LOG:
		return "PRINT_LOG";
	case SYSTEM_STATE_WAIT:
		return "WAIT";

	default:
		return "UNKNOWN";
	}
}

/* Convert plant status enum to readable text for UART logs */
static const char *PlantStatus_ToString(PlantStatus_t status){
	switch (status){
		case PLANT_STATUS_OK:
			return "OK";
		case PLANT_STATUS_LOW_SOIL:
			return "LOW_SOIL";
		case PLANT_STATUS_LOW_LIGHT:
			return "LOW_LIGHT";
		case PLANT_STATUS_LOW_SOIL_AND_LIGHT:
			return "LOW_SOIL_AND_LIGHT";
		case PLANT_STATUS_SENSOR_ERROR:
			return "SENSOR_ERROR";

        default:
            return "UNKNOWN";
	}
}

static void short_delay(void){
    for (volatile uint32_t i = 0; i < 50000; i++);
}

/* Convert plant status enum to user-friendly text for the 16x2 LCD */
static const char *PlantStatus_ToLCDString(PlantStatus_t status)
{
    switch (status)
    {
        case PLANT_STATUS_OK:
            return "I'm happy :)";

        case PLANT_STATUS_LOW_SOIL:
            return "Water me!";

        case PLANT_STATUS_LOW_LIGHT:
            return "Too dark!";

        case PLANT_STATUS_LOW_SOIL_AND_LIGHT:
            return "Help me!";

        case PLANT_STATUS_SENSOR_ERROR:
            return "Check sensors";

        default:
            return "Unknown state";
    }
}

static uint8_t ConvertRawToPercent(uint16_t raw_value, uint16_t raw_min, uint16_t raw_max){
	if (raw_value <= raw_min){
		return 0U;
	}
	if (raw_value >= raw_max){
		return 100U;
	}
	return (uint8_t)(((uint32_t)(raw_value - raw_min) * 100U) / (raw_max - raw_min));
}

/*
 * Calculate the plant status based on calibrated sensor thresholds.
 *
 * Soil:
 * - Higher raw value means drier soil.
 *
 * Light:
 * - Lower raw value means less light.
 */
static PlantStatus_t PlantMonitor_CalculateStatus(const PlantMonitorData_t *data){
    uint8_t is_soil_low = (data->soil_raw > SOIL_DRY_THRESHOLD);
    uint8_t is_light_low = (data->light_raw < LIGHT_LOW_THRESHOLD);

    if (is_soil_low && is_light_low){
        return PLANT_STATUS_LOW_SOIL_AND_LIGHT;
    }
    else if (is_soil_low){
        return PLANT_STATUS_LOW_SOIL;
    }
    else if (is_light_low){
        return PLANT_STATUS_LOW_LIGHT;
    }
    else{
        return PLANT_STATUS_OK;
    }
}

/*
 * Configure PA2 and PA3 for USART2.
 *
 * PA2 = USART2_TX
 * PA3 = USART2_RX
 * AF1 = USART2 alternate function on STM32G071.
 */
static void USART2_GPIO_Init(void)
{
    GPIO_Handle_t usart_gpio;

    usart_gpio.pGPIOx = GPIOA;
    usart_gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    usart_gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    usart_gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    usart_gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    usart_gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 1;

    /* PA2 -> USART2_TX */
    usart_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    GPIO_Init(&usart_gpio);

    /* PA3 -> USART2_RX */
    usart_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
    GPIO_Init(&usart_gpio);
}

/*
 * Configure USART2 for UART debug logs.
 *
 * Settings:
 * - Baud rate: 115200
 * - Data bits: 8
 * - Stop bits: 1
 * - Parity: none
 * - Mode: TX/RX
 */
static void USART2_Debug_Init(void){
	g_usart2_handle.pUSARTx = USART2;
	g_usart2_handle.USARTConfig.USART_Mode = USART_MODE_TXRX;
	g_usart2_handle.USARTConfig.USART_Baud = USART_STD_BAUD_115200;
	g_usart2_handle.USARTConfig.USART_NoOfStopBits = USART_STOPBITS_1;
	g_usart2_handle.USARTConfig.USART_WordLength = USART_WORDLEN_8BITS;
	g_usart2_handle.USARTConfig.USART_ParityControl = USART_PARITY_DISABLE;
	g_usart2_handle.USARTConfig.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;

	USART_Init(&g_usart2_handle);
}

static void UART_Log(const char *msg){
	USART_SendData(&g_usart2_handle, (uint8_t *)msg, strlen(msg));
}

static void ADC_GPIO_Init(void){
	GPIO_Handle_t adc_gpio;

	adc_gpio.pGPIOx = GPIOA;

	adc_gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
	adc_gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
	adc_gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	adc_gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	adc_gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 0;

    /* PA0 -> ADC channel 0, LDR light sensor */
	adc_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GPIO_Init(&adc_gpio);

    /* PA1 -> ADC channel 1, soil moisture sensor */
	adc_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_1;
	GPIO_Init(&adc_gpio);
}

static void UserButton_GPIO_Init(void){
    GPIO_Handle_t button_gpio;

    button_gpio.pGPIOx = GPIOC;
    button_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    button_gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    button_gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    button_gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    button_gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    button_gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 0;

    GPIO_Init(&button_gpio);
}

static uint8_t UserButton_WasPressed(void){
	static uint8_t last_button_state = GPIO_PIN_SET;

	uint8_t current_button_state = GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13);
	uint8_t was_pressed = 0U;

    /*
     * The onboard user button is active-low:
     * Released = SET
     * Pressed = RESET
     * Detect only the transition from released to pressed.
     */
	if ((last_button_state == GPIO_PIN_SET) && (current_button_state == GPIO_PIN_RESET)){
		was_pressed = 1U;
	}
	last_button_state = current_button_state;
	return was_pressed;
}

static void I2C1_GPIO_Init(void){
	GPIO_Handle_t I2CPins;
	memset(&I2CPins, 0, sizeof(I2CPins));

	I2CPins.pGPIOx = GPIOB;

	I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
	I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PU;
	I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = 6U;

	/* PB8 -> I2C1_SCL */
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_8;
	GPIO_Init(&I2CPins);

	 /* PB9 -> I2C1_SDA */
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
	GPIO_Init(&I2CPins);
}

static void I2C1_LCD_Init(void){
    memset(&g_i2c1_handle, 0, sizeof(g_i2c1_handle));

    g_i2c1_handle.pI2Cx = I2C1;

    g_i2c1_handle.I2CConfig.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    g_i2c1_handle.I2CConfig.I2C_DeviceAddress = 0x61U;
    g_i2c1_handle.I2CConfig.I2C_ACKControl = I2C_ACK_ENABLE;
    g_i2c1_handle.I2CConfig.I2C_FMDutyCycle = I2C_FM_DUTY_2;

    I2C_Init(&g_i2c1_handle);
}

/* Print the current system state over UART */
static void Log_CurrentState(SystemState_t state){
	UART_Log("[STATE] ");
	UART_Log(SystemState_ToString(state));
	UART_Log("\r\n");
}

static void AlertLEDs_GPIO_Init(void)
{
    GPIO_Handle_t led_gpio;

    led_gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    led_gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    led_gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    led_gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    /* Red LED - PA10 */
    led_gpio.pGPIOx = RED_LED_GPIO_PORT;
    led_gpio.GPIO_PinConfig.GPIO_PinNumber = RED_LED_GPIO_PIN;
    GPIO_Init(&led_gpio);

    /* Yellow LED - PB3 */
    led_gpio.pGPIOx = YELLOW_LED_GPIO_PORT;
    led_gpio.GPIO_PinConfig.GPIO_PinNumber = YELLOW_LED_GPIO_PIN;
    GPIO_Init(&led_gpio);

    /* Green LED - PB5 */
    led_gpio.pGPIOx = GREEN_LED_GPIO_PORT;
    led_gpio.GPIO_PinConfig.GPIO_PinNumber = GREEN_LED_GPIO_PIN;
    GPIO_Init(&led_gpio);

    GPIO_WriteToOutputPin(RED_LED_GPIO_PORT, RED_LED_GPIO_PIN, GPIO_PIN_RESET);
    GPIO_WriteToOutputPin(YELLOW_LED_GPIO_PORT, YELLOW_LED_GPIO_PIN, GPIO_PIN_RESET);
    GPIO_WriteToOutputPin(GREEN_LED_GPIO_PORT, GREEN_LED_GPIO_PIN, GPIO_PIN_RESET);
}

static void AlertLEDs_Update(PlantStatus_t status){
    GPIO_WriteToOutputPin(RED_LED_GPIO_PORT, RED_LED_GPIO_PIN, GPIO_PIN_RESET);
    GPIO_WriteToOutputPin(YELLOW_LED_GPIO_PORT, YELLOW_LED_GPIO_PIN, GPIO_PIN_RESET);
    GPIO_WriteToOutputPin(GREEN_LED_GPIO_PORT, GREEN_LED_GPIO_PIN, GPIO_PIN_RESET);

    switch(status){
    	case PLANT_STATUS_OK:
    		GPIO_WriteToOutputPin(GREEN_LED_GPIO_PORT, GREEN_LED_GPIO_PIN, GPIO_PIN_SET);
    		break;

    	case PLANT_STATUS_LOW_SOIL:
    	case PLANT_STATUS_LOW_LIGHT:
    		GPIO_WriteToOutputPin(YELLOW_LED_GPIO_PORT, YELLOW_LED_GPIO_PIN, GPIO_PIN_SET);
    		break;

    	case PLANT_STATUS_LOW_SOIL_AND_LIGHT:
    	case PLANT_STATUS_SENSOR_ERROR:
    		GPIO_WriteToOutputPin(RED_LED_GPIO_PORT, RED_LED_GPIO_PIN, GPIO_PIN_SET);
    		break;

        default:
            GPIO_WriteToOutputPin(RED_LED_GPIO_PORT, RED_LED_GPIO_PIN, GPIO_PIN_SET);
            break;
    }
}

static void LCD_UpdateDisplay(void){
    char line1[17];
    char line2[17];

    if (g_lcd_page == 0U){
        snprintf(line1, sizeof(line1), "Moisture:%3u%%   ", g_plant_data.soil_percent);
        snprintf(line2, sizeof(line2), "Light:%3u%%      ", g_plant_data.light_percent);
    }
    else if (g_lcd_page == 1U){
        if (g_plant_data.bme280_is_available){
            snprintf(line1, sizeof(line1), "Temp:%2ld.%02ldC   ", g_plant_data.air_temperature_c_x100 / 100, g_plant_data.air_temperature_c_x100 % 100);
            snprintf(line2, sizeof(line2), "Hum:%2lu.%02lu%%    ", g_plant_data.air_humidity_percent_x100 / 100U, g_plant_data.air_humidity_percent_x100 % 100U);
        }
        else{
            snprintf(line1, sizeof(line1), "%-16s", "Env sensor:");
            snprintf(line2, sizeof(line2), "%-16s", "Not available");
        }
    }
    else{
        snprintf(line1, sizeof(line1), "%-16s", "Plant says:");
        snprintf(line2, sizeof(line2), "%-16s", PlantStatus_ToLCDString(g_plant_data.plant_status));
    }

    LCD_SetCursor(0, 0);
    LCD_Print(line1);

    LCD_SetCursor(1, 0);
    LCD_Print(line2);
}

static void LCD_HandlePageButton(void){
    if (UserButton_WasPressed()){
        g_lcd_page++;

        if (g_lcd_page >= LCD_PAGE_COUNT){
            g_lcd_page = 0U;
        }

        /*
         * Update the LCD immediately after changing the page,
         * so the user sees the result of the button press right away.
         */
        LCD_UpdateDisplay();

        UART_Log("[BUTTON] LCD page changed\r\n");
    }
}

static void BME280_Application_Init(void){
    uint8_t chip_id = 0U;
    BME280_CalibData_t calib_data;

    BME280_Init(&g_i2c1_handle, BME280_I2C_ADDR);

    UART_Log("[BME280] Initializing environmental sensor\r\n");

    if (!BME280_ReadChipID(&chip_id)){
        UART_Log("[BME280] Chip ID read failed\r\n");
        g_plant_data.bme280_is_available = 0U;
        return;
    }

    if (chip_id != BME280_CHIP_ID_VALUE){
        UART_Log("[BME280] Unexpected chip ID\r\n");
        g_plant_data.bme280_is_available = 0U;
        return;
    }

    UART_Log("[BME280] Sensor detected successfully\r\n");

    if (!BME280_ReadCalibrationData(&calib_data)){
        UART_Log("[BME280] Calibration data read failed\r\n");
        g_plant_data.bme280_is_available = 0U;
        return;
    }

    if (!BME280_ConfigureMeasurement()){
        UART_Log("[BME280] Measurement configuration failed\r\n");
        g_plant_data.bme280_is_available = 0U;
        return;
    }

    g_plant_data.bme280_is_available = 1U;
    UART_Log("[BME280] Environmental sensor ready\r\n");
}

static void BME280_ReadEnvironment(void){
    BME280_CompensatedData_t env_data;

    if (g_plant_data.bme280_is_available == 0U){
        return;
    }

    if (BME280_ReadTemperatureHumidity(&env_data)){
        g_plant_data.air_temperature_c_x100 = env_data.temperature_c_x100;
        g_plant_data.air_humidity_percent_x100 = env_data.humidity_percent_x100;
    }
    else{
        UART_Log("[BME280] Environment read failed\r\n");
        g_plant_data.bme280_is_available = 0U;
    }
}

static void DS3231_Application_Init(void){
    DS3231_DateTime_t date_time;

    DS3231_Init(&g_i2c1_handle, DS3231_I2C_ADDR);

    UART_Log("[DS3231] Initializing RTC\r\n");

    if (DS3231_GetDateTime(&date_time)){
        g_plant_data.timestamp = date_time;
        g_plant_data.rtc_is_available = 1U;
        UART_Log("[DS3231] RTC ready\r\n");
    }
    else{
        g_plant_data.rtc_is_available = 0U;
        UART_Log("[DS3231] RTC read failed\r\n");
    }
}

static void DS3231_ReadTimestamp(void){
    DS3231_DateTime_t date_time;

    if (g_plant_data.rtc_is_available == 0U){
        return;
    }

    if (DS3231_GetDateTime(&date_time)){
        g_plant_data.timestamp = date_time;
    }
    else{
        g_plant_data.rtc_is_available = 0U;
        UART_Log("[DS3231] Timestamp read failed\r\n");
    }
}

static void SPI1_GPIO_Init(void){
    GPIO_Handle_t spi_gpio;

    memset(&spi_gpio, 0, sizeof(spi_gpio));

    spi_gpio.pGPIOx = GPIOA;
    spi_gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    spi_gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
    spi_gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    spi_gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;

    /*
     * STM32G071:
     * PA5 = SPI1_SCK
     * PA6 = SPI1_MISO
     * PA7 = SPI1_MOSI
     * On STM32G071 these SPI1 pins use AF0
     */
    spi_gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    /* PA5 -> SPI1_SCK */
    spi_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
    GPIO_Init(&spi_gpio);

    /* PA6 -> SPI1_MISO */
    spi_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    GPIO_Init(&spi_gpio);

    /* PA7 -> SPI1_MOSI */
    spi_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
    GPIO_Init(&spi_gpio);
}

static void SD_CS_GPIO_Init(void){
    GPIO_Handle_t cs_gpio;

    memset(&cs_gpio, 0, sizeof(cs_gpio));

    cs_gpio.pGPIOx = SD_CS_GPIO_PORT;
    cs_gpio.GPIO_PinConfig.GPIO_PinNumber = SD_CS_GPIO_PIN;
    cs_gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    cs_gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
    cs_gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    cs_gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    cs_gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&cs_gpio);

    /*
     * CS is active-low
     * Keep the card deselected when we are not talking to it
     */
    GPIO_WriteToOutputPin(SD_CS_GPIO_PORT, SD_CS_GPIO_PIN, GPIO_PIN_SET);
}

static void SPI1_SD_Init(void){
    memset(&g_spi1_handle, 0, sizeof(g_spi1_handle));

    g_spi1_handle.pSPIx = SPI1;

    /*
     * SD card initialization should start with a slow SPI clock
     * DIV256 is slow and safe for bring-up
     */
    g_spi1_handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
    g_spi1_handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
    g_spi1_handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV256;
    g_spi1_handle.SPIConfig.SPI_DataSize = SPI_DS_8BITS;
    g_spi1_handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
    g_spi1_handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
    g_spi1_handle.SPIConfig.SPI_SSM = SPI_SSM_EN;

    SPI_Init(&g_spi1_handle);
    SPI_PeripheralControl(SPI1, ENABLE);
}

static void SD_CardFullInit_Test(void)
{
    uint8_t init_ok = 0U;

    UART_Log("[SD] Initializing SPI interface\r\n");

    SPI1_GPIO_Init();
    SD_CS_GPIO_Init();
    SPI1_SD_Init();

    SD_Card_Init(&g_spi1_handle, SD_CS_GPIO_PORT, SD_CS_GPIO_PIN);

    UART_Log("[SD] Initializing card\r\n");

    init_ok = SD_Card_InitCard();

    if (init_ok)
    {
        UART_Log("[SD] Card initialized successfully\r\n");

        if (SD_Card_GetType() == SD_CARD_TYPE_SDHC)
        {
            UART_Log("[SD] Card type: SDHC\r\n");
        }
        else
        {
            UART_Log("[SD] Card type: unknown\r\n");
        }
    }
    else
    {
        UART_Log("[SD] Card initialization failed\r\n");
    }
}

static void SD_ReadBlock_Test(void)
{
    uint8_t block_buffer[512];
    char log_buffer[100];

    UART_Log("[SD] Reading block 2048\r\n");

    if (SD_Card_ReadBlock(2048U, block_buffer))
    {
        UART_Log("[SD] Block read successfully\r\n");

        snprintf(log_buffer,
                 sizeof(log_buffer),
                 "[SD] First 16 bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                 block_buffer[0],
                 block_buffer[1],
                 block_buffer[2],
                 block_buffer[3],
                 block_buffer[4],
                 block_buffer[5],
                 block_buffer[6],
                 block_buffer[7],
                 block_buffer[8],
                 block_buffer[9],
                 block_buffer[10],
                 block_buffer[11],
                 block_buffer[12],
                 block_buffer[13],
                 block_buffer[14],
                 block_buffer[15]);

        UART_Log(log_buffer);
    }
    else
    {
        UART_Log("[SD] Block read failed\r\n");
    }
}

static void SD_WriteBlock_Test(void)
{
    uint8_t original_block[512];
    uint8_t write_block[512];
    uint8_t verify_block[512];

    uint8_t test_ok = 1U;

    UART_Log("[SD] Write block test started\r\n");

    /*
     * Step 1:
     * Read original block so we can restore it after the test.
     */
    UART_Log("[SD] Backing up test block\r\n");

    if (!SD_Card_ReadBlock(SD_TEST_BLOCK_ADDR, original_block))
    {
        UART_Log("[SD] Failed to read original test block\r\n");
        return;
    }

    /*
     * Step 2:
     * Prepare a test pattern.
     */
    for (uint16_t i = 0U; i < 512U; i++)
    {
        write_block[i] = 0xA5U;
    }

    write_block[0] = 'P';
    write_block[1] = 'L';
    write_block[2] = 'A';
    write_block[3] = 'N';
    write_block[4] = 'T';
    write_block[5] = '_';
    write_block[6] = 'S';
    write_block[7] = 'D';
    write_block[8] = '_';
    write_block[9] = 'T';
    write_block[10] = 'E';
    write_block[11] = 'S';
    write_block[12] = 'T';

    /*
     * Step 3:
     * Write the test pattern.
     */
    UART_Log("[SD] Writing test pattern\r\n");

    if (!SD_Card_WriteBlock(SD_TEST_BLOCK_ADDR, write_block))
    {
        UART_Log("[SD] Test block write failed\r\n");
        return;
    }

    /*
     * Step 4:
     * Read the same block back.
     */
    UART_Log("[SD] Reading test block back\r\n");

    if (!SD_Card_ReadBlock(SD_TEST_BLOCK_ADDR, verify_block))
    {
        UART_Log("[SD] Failed to read test block back\r\n");
        return;
    }

    /*
     * Step 5:
     * Verify first 512 bytes match exactly.
     */
    for (uint16_t i = 0U; i < 512U; i++)
    {
        if (verify_block[i] != write_block[i])
        {
            test_ok = 0U;
            break;
        }
    }

    if (test_ok)
    {
        UART_Log("[SD] Write/read verify OK\r\n");
    }
    else
    {
        UART_Log("[SD] Write/read verify FAILED\r\n");
    }

    /*
     * Step 6:
     * Restore the original block.
     */
    UART_Log("[SD] Restoring original block\r\n");

    if (!SD_Card_WriteBlock(SD_TEST_BLOCK_ADDR, original_block))
    {
        UART_Log("[SD] Failed to restore original block\r\n");
        return;
    }

    UART_Log("[SD] Original block restored\r\n");
}

static void FatFs_Test(void){
    static FATFS fs;
    static FIL file;

    FRESULT result;
    UINT bytes_written = 0U;
    const char test_text[] = "Hello from STM32 Smart Plant Monitor\r\n";

    UART_Log("[FS] Mounting filesystem\r\n");

    /*
     * Mount drive 0
     * The empty string "" means the default logical drive
     */
    result = f_mount(&fs, "", 1U);

    if (result != FR_OK){
        char log_buffer[60];
        snprintf(log_buffer, sizeof(log_buffer), "[FS] Mount failed, error=%d\r\n", result);
        UART_Log(log_buffer);
        return;
    }

    UART_Log("[FS] Filesystem mounted successfully\r\n");

    /* Create or overwrite a simple test file */
    result = f_open(&file, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);

    if (result != FR_OK){
        char log_buffer[60];
        snprintf(log_buffer, sizeof(log_buffer), "[FS] f_open failed, error=%d\r\n", result);
        UART_Log(log_buffer);
        return;
    }

    UART_Log("[FS] test.txt opened successfully\r\n");

    result = f_write(&file, test_text, sizeof(test_text) - 1U, &bytes_written);

    if ((result != FR_OK) || (bytes_written != (sizeof(test_text) - 1U))){
        char log_buffer[80];
        snprintf(log_buffer, sizeof(log_buffer), "[FS] f_write failed, error=%d, written=%lu\r\n", result, (unsigned long)bytes_written);
        UART_Log(log_buffer);
        f_close(&file);
        return;
    }

    UART_Log("[FS] test.txt written successfully\r\n");
    f_close(&file);
    UART_Log("[FS] test.txt closed successfully\r\n");
}

int main(void){
	USART2_GPIO_Init();
    USART2_Debug_Init();

    AlertLEDs_GPIO_Init();
    UserButton_GPIO_Init();

    I2C1_GPIO_Init();
    I2C1_LCD_Init();

    LCD_Init(&g_i2c1_handle, LCD_I2C_ADDR);

    BME280_Application_Init();
    DS3231_Application_Init();
    SD_CardFullInit_Test();
    FatFs_Test();

    ADC_GPIO_Init();
    ADC_Init();

    UART_Log("[ADC] ADC initialized and ready\r\n");

    g_current_state = SYSTEM_STATE_INIT;
    Log_CurrentState(g_current_state);

    g_current_state = SYSTEM_STATE_READ_SENSORS;

    while (1)
    {
    	Log_CurrentState(g_current_state);

        switch (g_current_state)
        {
            case SYSTEM_STATE_READ_SENSORS:
            	DS3231_ReadTimestamp();
            	g_plant_data.soil_raw = ADC_ReadChannel(ADC_CHANNEL_1);
            	g_plant_data.light_raw = ADC_ReadChannel(ADC_CHANNEL_0);
            	BME280_ReadEnvironment();

            	UART_Log("[SENSORS] Timestamp, soil, light and environment updated\r\n");

                g_current_state = SYSTEM_STATE_PROCESS_DATA;
                break;

            case SYSTEM_STATE_PROCESS_DATA:
            {
                g_plant_data.light_percent = ConvertRawToPercent(g_plant_data.light_raw, LIGHT_DARK_RAW, LIGHT_BRIGHT_RAW);
                g_plant_data.soil_percent = 100U - ConvertRawToPercent(g_plant_data.soil_raw, SOIL_WET_RAW, SOIL_DRY_RAW);

                g_plant_data.plant_status = PlantMonitor_CalculateStatus(&g_plant_data);
                UART_Log("[PROCESS] Plant status updated\r\n");

                g_current_state = SYSTEM_STATE_UPDATE_ALERTS;
                break;
            }

            case SYSTEM_STATE_UPDATE_ALERTS:
            	AlertLEDs_Update(g_plant_data.plant_status);

                UART_Log("[ALERT] Plant status = ");
                UART_Log(PlantStatus_ToString(g_plant_data.plant_status));
                UART_Log("\r\n");

                g_current_state = SYSTEM_STATE_UPDATE_DISPLAY;
                break;

            case SYSTEM_STATE_UPDATE_DISPLAY:
            	LCD_UpdateDisplay();

                UART_Log("[DISPLAY] LCD updated\r\n");

                g_current_state = SYSTEM_STATE_PRINT_LOG;
                break;

            case SYSTEM_STATE_PRINT_LOG:
            {
                char log_buffer[220];

                if (g_plant_data.rtc_is_available && g_plant_data.bme280_is_available){
                    snprintf(log_buffer,
                             sizeof(log_buffer),
                             "[LOG] 20%02u-%02u-%02u %02u:%02u:%02u | soil_raw=%u, soil=%u%%, light_raw=%u, light=%u%%, temp=%ld.%02ldC, hum=%lu.%02lu%%, status=%s\r\n",
                             g_plant_data.timestamp.year,
                             g_plant_data.timestamp.month,
                             g_plant_data.timestamp.date,
                             g_plant_data.timestamp.hours,
                             g_plant_data.timestamp.minutes,
                             g_plant_data.timestamp.seconds,
                             g_plant_data.soil_raw,
                             g_plant_data.soil_percent,
                             g_plant_data.light_raw,
                             g_plant_data.light_percent,
                             g_plant_data.air_temperature_c_x100 / 100,
                             g_plant_data.air_temperature_c_x100 % 100,
                             g_plant_data.air_humidity_percent_x100 / 100U,
                             g_plant_data.air_humidity_percent_x100 % 100U,
                             PlantStatus_ToString(g_plant_data.plant_status));
                }
                else if (g_plant_data.rtc_is_available){
                    snprintf(log_buffer,
                             sizeof(log_buffer),
                             "[LOG] 20%02u-%02u-%02u %02u:%02u:%02u | soil_raw=%u, soil=%u%%, light_raw=%u, light=%u%%, env=N/A, status=%s\r\n",
                             g_plant_data.timestamp.year,
                             g_plant_data.timestamp.month,
                             g_plant_data.timestamp.date,
                             g_plant_data.timestamp.hours,
                             g_plant_data.timestamp.minutes,
                             g_plant_data.timestamp.seconds,
                             g_plant_data.soil_raw,
                             g_plant_data.soil_percent,
                             g_plant_data.light_raw,
                             g_plant_data.light_percent,
                             PlantStatus_ToString(g_plant_data.plant_status));
                }
                else if (g_plant_data.bme280_is_available){
                    snprintf(log_buffer,
                             sizeof(log_buffer),
                             "[LOG] soil_raw=%u, soil=%u%%, light_raw=%u, light=%u%%, temp=%ld.%02ldC, hum=%lu.%02lu%%, status=%s\r\n",
                             g_plant_data.soil_raw,
                             g_plant_data.soil_percent,
                             g_plant_data.light_raw,
                             g_plant_data.light_percent,
                             g_plant_data.air_temperature_c_x100 / 100,
                             g_plant_data.air_temperature_c_x100 % 100,
                             g_plant_data.air_humidity_percent_x100 / 100U,
                             g_plant_data.air_humidity_percent_x100 % 100U,
                             PlantStatus_ToString(g_plant_data.plant_status));
                }
                else{
                    snprintf(log_buffer,
                             sizeof(log_buffer),
                             "[LOG] soil_raw=%u, soil=%u%%, light_raw=%u, light=%u%%, env=N/A, status=%s\r\n",
                             g_plant_data.soil_raw,
                             g_plant_data.soil_percent,
                             g_plant_data.light_raw,
                             g_plant_data.light_percent,
                             PlantStatus_ToString(g_plant_data.plant_status));
                }

                UART_Log(log_buffer);

                g_current_state = SYSTEM_STATE_WAIT;
                break;
            }
            case SYSTEM_STATE_WAIT:
            {
                UART_Log("[WAIT] Waiting before next measurement cycle\r\n");

                for (uint8_t i = 0; i < 10U; i++){
                    LCD_HandlePageButton();
                    short_delay();
                }

                g_current_state = SYSTEM_STATE_READ_SENSORS;
                break;
            }

            default:
                g_current_state = SYSTEM_STATE_READ_SENSORS;
                break;
        }
    }
}
