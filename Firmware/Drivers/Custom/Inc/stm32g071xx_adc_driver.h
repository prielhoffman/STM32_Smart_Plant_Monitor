#ifndef INC_STM32G071XX_ADC_DRIVER_H_
#define INC_STM32G071XX_ADC_DRIVER_H_

#include "stm32g071xx.h"

/*
 * ADC channel definitions.
 * For the first bring-up:
 * - PA0 is connected to ADC channel 0.
 * - PA1 is connected to ADC channel 1.
 */
#define ADC_CHANNEL_0        0U
#define ADC_CHANNEL_1        1U
#define ADC_CHANNEL_0        0U
#define ADC_CHANNEL_1        1U

/* ADC driver APIs */
void ADC_PeriClockControl(uint8_t EnOrDi);
void ADC_Init(void);
uint16_t ADC_ReadChannel(uint8_t channel);

#endif
