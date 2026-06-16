#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_usart_driver.h"
#include "stm32g071xx_adc_driver.h"

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

    PlantStatus_t plant_status;
} PlantMonitorData_t;

static USART_Handle_t g_usart2_handle;
static PlantMonitorData_t g_plant_data;
static SystemState_t g_current_state = SYSTEM_STATE_INIT;

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
 * Simple blocking delay.
 * This delay is used only for the initial board bring-up test.
 */
static void delay(void){
	for (volatile uint32_t i = 0; i < 300000; i++);
}

/*
 * Configure PA2 and PA3 for USART2.
 *
 * PA2 = USART2_TX
 * PA3 = USART2_RX
 * AF1 = USART2 alternate function on STM32G071.
 */
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

/* Print the current system state over UART */
static void Log_CurrentState(SystemState_t state){
	UART_Log("[STATE] ");
	UART_Log(SystemState_ToString(state));
	UART_Log("\r\n");
}

int main(void)
{
    /*
     * LD2 on the NUCLEO-G071RB board is connected to PA5.
     * This test verifies that the custom GPIO driver can configure and toggle a GPIO output pin.
     */
    GPIO_Handle_t led_gpio;

    led_gpio.pGPIOx = GPIOA;
    led_gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
    led_gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    led_gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_VERY_LOW;
    led_gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    led_gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    led_gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 0;

    GPIO_Init(&led_gpio);

    USART2_GPIO_Init();
    USART2_Debug_Init();

    ADC_GPIO_Init();
    ADC_Init();

    UART_Log("[ADC] ADC initialized and ready\r\n");

    g_current_state = SYSTEM_STATE_INIT;
    Log_CurrentState(g_current_state);

    g_current_state = SYSTEM_STATE_READ_SENSORS;

    while (1)
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);

        Log_CurrentState(g_current_state);

        switch (g_current_state)
        {
            case SYSTEM_STATE_READ_SENSORS:
            	g_plant_data.soil_raw = ADC_ReadChannel(ADC_CHANNEL_1);
            	g_plant_data.light_raw = ADC_ReadChannel(ADC_CHANNEL_0);
            	UART_Log("[SENSORS] Soil and light ADC updated\r\n");

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
                UART_Log("[ALERT] Plant status = ");
                UART_Log(PlantStatus_ToString(g_plant_data.plant_status));
                UART_Log("\r\n");

                g_current_state = SYSTEM_STATE_UPDATE_DISPLAY;
                break;

            case SYSTEM_STATE_UPDATE_DISPLAY:
                UART_Log("[DISPLAY] LCD update skipped - not connected yet\r\n");

                g_current_state = SYSTEM_STATE_PRINT_LOG;
                break;

            case SYSTEM_STATE_PRINT_LOG:
            {
                char log_buffer[100];

                snprintf(log_buffer, sizeof(log_buffer), "[LOG] soil_raw=%u, soil=%u%%, light_raw=%u, light=%u%%, status=%s\r\n",
                         g_plant_data.soil_raw, g_plant_data.soil_percent, g_plant_data.light_raw, g_plant_data.light_percent, PlantStatus_ToString(g_plant_data.plant_status));

                UART_Log(log_buffer);

                g_current_state = SYSTEM_STATE_WAIT;
                break;
            }

            case SYSTEM_STATE_WAIT:
                UART_Log("[WAIT] Waiting before next measurement cycle\r\n");

                delay();

                g_current_state = SYSTEM_STATE_READ_SENSORS;
                break;

            default:
                g_current_state = SYSTEM_STATE_READ_SENSORS;
                break;
        }
    }
}
