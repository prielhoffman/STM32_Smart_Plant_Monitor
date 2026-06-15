#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_usart_driver.h"

#include <stdint.h>
#include <string.h>

static USART_Handle_t g_usart2_handle;

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

    while (1){
    	GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
        UART_Log("[INIT] Smart Plant Care Monitor started\r\n");
    	delay();
    }
}
