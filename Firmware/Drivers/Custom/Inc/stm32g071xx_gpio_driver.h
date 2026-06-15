#ifndef INC_STM32G071XX_GPIO_DRIVER_H_
#define INC_STM32G071XX_GPIO_DRIVER_H_

#include "stm32g071xx.h"

/* ------------------------------------------------------------
 * Configuration structures
 * ------------------------------------------------------------ */

typedef struct
{
    uint8_t GPIO_PinNumber;      /* GPIO pin number: 0 to 15 */
    uint8_t GPIO_PinMode;        /* GPIO mode: input, output, alternate function, analog, or interrupt mode */
    uint8_t GPIO_PinSpeed;       /* GPIO output speed: very low, low, high, or very high */
    uint8_t GPIO_PinPuPdControl; /* GPIO pull-up / pull-down configuration */
    uint8_t GPIO_PinOPType;      /* GPIO output type: push-pull or open-drain */
    uint8_t GPIO_PinAltFunMode;  /* GPIO alternate function mode: AF0 to AF15 */
} GPIO_PinConfig_t;

typedef struct
{
    GPIO_RegDef_t *pGPIOx;           /* Pointer to the GPIO port register definition structure */
    GPIO_PinConfig_t GPIO_PinConfig; /* Configuration settings for the selected GPIO pin */
} GPIO_Handle_t;

/* ------------------------------------------------------------
 * GPIO pin numbers
 * ------------------------------------------------------------ */

#define GPIO_PIN_NO_0       0U
#define GPIO_PIN_NO_1       1U
#define GPIO_PIN_NO_2       2U
#define GPIO_PIN_NO_3       3U
#define GPIO_PIN_NO_4       4U
#define GPIO_PIN_NO_5       5U
#define GPIO_PIN_NO_6       6U
#define GPIO_PIN_NO_7       7U
#define GPIO_PIN_NO_8       8U
#define GPIO_PIN_NO_9       9U
#define GPIO_PIN_NO_10      10U
#define GPIO_PIN_NO_11      11U
#define GPIO_PIN_NO_12      12U
#define GPIO_PIN_NO_13      13U
#define GPIO_PIN_NO_14      14U
#define GPIO_PIN_NO_15      15U

/* ------------------------------------------------------------
 * GPIO pin modes
 * ------------------------------------------------------------ */

/*
 * Non-interrupt modes match GPIOx_MODER:
 * 00: input
 * 01: output
 * 10: alternate function
 * 11: analog
 */
#define GPIO_MODE_IN        0U
#define GPIO_MODE_OUT       1U
#define GPIO_MODE_ALTFN     2U
#define GPIO_MODE_ANALOG    3U
#define GPIO_MODE_IT_FT     4U   /* Interrupt on falling edge */
#define GPIO_MODE_IT_RT     5U   /* Interrupt on rising edge */
#define GPIO_MODE_IT_RFT    6U   /* Interrupt on rising and falling edge */

/* ------------------------------------------------------------
 * GPIO output types
 * ------------------------------------------------------------ */

#define GPIO_OP_TYPE_PP     0U   /* Push-pull */
#define GPIO_OP_TYPE_OD     1U   /* Open-drain */

/* ------------------------------------------------------------
 * GPIO output speeds
 * ------------------------------------------------------------ */

/*
 * GPIOx_OSPEEDR:
 * 00: Very low speed
 * 01: Low speed
 * 10: High speed
 * 11: Very high speed
 */
#define GPIO_SPEED_VERY_LOW     0U
#define GPIO_SPEED_LOW          1U
#define GPIO_SPEED_HIGH         2U
#define GPIO_SPEED_VERY_HIGH    3U

/* ------------------------------------------------------------
 * GPIO pull-up / pull-down configuration
 * ------------------------------------------------------------ */

/*
 * GPIOx_PUPDR:
 * 00: No pull-up, no pull-down
 * 01: Pull-up
 * 10: Pull-down
 * 11: Reserved
 */
#define GPIO_NO_PUPD        0U
#define GPIO_PU             1U
#define GPIO_PD             2U

/* ------------------------------------------------------------
 * GPIO driver APIs
 * ------------------------------------------------------------ */

void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnOrDi);

void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);

void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value);
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value);
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);

void GPIO_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi);
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
void GPIO_IRQHandling(uint8_t PinNumber);

#endif
