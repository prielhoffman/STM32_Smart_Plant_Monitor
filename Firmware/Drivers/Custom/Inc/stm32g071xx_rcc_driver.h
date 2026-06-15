#ifndef INC_STM32G071XX_RCC_DRIVER_H_
#define INC_STM32G071XX_RCC_DRIVER_H_

#include "stm32g071xx.h"

/* ------------------------------------------------------------
 * RCC driver APIs
 * ------------------------------------------------------------ */

/* Return the APB1 peripheral clock frequency in Hz.
 * Used by peripherals such as USART2/3/4/5, SPI2/3 and I2C.
 */
uint32_t RCC_GetPCLK1Value(void);

/*
 * Return the APB2 peripheral clock frequency in Hz.
 * Used by peripherals such as USART1, USART6 and SPI1.
 */
uint32_t RCC_GetPCLK2Value(void);

#endif
