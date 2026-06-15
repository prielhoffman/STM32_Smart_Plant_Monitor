#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include <stdint.h>

/*
 * Simple blocking delay.
 * This delay is used only for the initial board bring-up test.
 */
void delay(void){
	for (volatile uint32_t i = 0; i < 300000; i++);
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

    while (1){
    	GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
    	delay();
    }
}
