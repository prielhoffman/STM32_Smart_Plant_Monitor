#include "stm32g071xx_adc_driver.h"

/*
 * Small delay used after enabling the ADC voltage regulator.
 * The regulator needs a short stabilization time before calibration.
 */
static void ADC_RegulatorDelay(void){
    for (volatile uint32_t i = 0; i < 1000; i++);
}

/* Enable or disable the peripheral clock for ADC */
void ADC_PeriClockControl(uint8_t EnOrDi){
	if (EnOrDi == ENABLE){
		ADC_PCLK_EN();
	}
	else{
		ADC_PCLK_DI();
	}
}

void ADC_Init(void){
	ADC_PeriClockControl(ENABLE);

	/* Enable ADC internal voltage regulator */
	ADC->CR |= (1U << ADC_CR_ADVREGEN);
	ADC_RegulatorDelay();

	/* Start ADC calibration */
	ADC->CR |= (1U << ADC_CR_ADCAL);

	/* Wait until calibration is complete */
	while (ADC->CR & (1U << ADC_CR_ADCAL));

	/* Clear ADC ready flag before enabling */
	ADC->ISR |= ADC_ADRDY_FLAG;

	/* Enable ADC */
	ADC->CR |= (1U << ADC_CR_ADEN);

	/* Wait until ADC is ready */
	while(!(ADC->ISR & ADC_ADRDY_FLAG));
}

/*
 * Read one ADC channel using single conversion polling mode.
 * The ADC must already be initialized and enabled before calling this function.
 */
uint16_t ADC_ReadChannel(uint8_t channel){
	uint16_t adc_value = 0;

	/* Make sure no conversion is currently running before changing channel */
	while (ADC->CR & (1U << ADC_CR_ADSTART));

	/* Select ADC channel */
	ADC->CHSELR = (1U << channel);

    /* Wait until the ADC channel configuration is updated */
	while (!(ADC->ISR & ADC_CCRDY_FLAG));
	ADC->ISR |= ADC_CCRDY_FLAG;

    /* Clear old conversion flags before starting a new conversion */
    ADC->ISR |= ADC_EOC_FLAG;
    ADC->ISR |= ADC_EOS_FLAG;

    /* Start ADC conversion */
	ADC->CR |= (1U << ADC_CR_ADSTART);

    /* Wait until conversion is complete */
	while (!(ADC->ISR & ADC_EOC_FLAG));

    /* Read conversion result */
	adc_value = (uint16_t)(ADC->DR & 0x0FFFU);

	return adc_value;
}
