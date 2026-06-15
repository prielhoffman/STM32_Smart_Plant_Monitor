#include "stm32g071xx_rcc_driver.h"

/* ------------------------------------------------------------
 * Clock source values
 * ------------------------------------------------------------ */

#define HSI16_VALUE     16000000U
#define HSE_VALUE       8000000U
#define LSI_VALUE       32000U
#define LSE_VALUE       32768U

/* ------------------------------------------------------------
 * Prescaler lookup tables
 * ------------------------------------------------------------ */

/*
 * AHB prescaler table.
 *
 * RCC_CFGR HPRE encoding:
 * 0xxx: /1
 * 1000: /2
 * 1001: /4
 * 1010: /8
 * 1011: /16
 * 1100: /64
 * 1101: /128
 * 1110: /256
 * 1111: /512
 */
static const uint16_t AHB_PreScaler[8] = {2U, 4U, 8U, 16U, 64U, 128U, 256U, 512U};

/*
 * APB prescaler table.
 *
 * RCC_CFGR PPRE encoding:
 * 0xx: /1
 * 100: /2
 * 101: /4
 * 110: /8
 * 111: /16
 */
static const uint8_t APB_PreScaler[4] = {2U, 4U, 8U, 16U};

/* ------------------------------------------------------------
 * Static helper functions
 * ------------------------------------------------------------ */

/*
 * Return the current system clock frequency.
 *
 * Note:
 * PLL clock calculation is not implemented yet. If PLL is selected,
 * the function currently falls back to HSI16_VALUE.
 */
static uint32_t RCC_GetSYSCLKValue(void){
    uint32_t sysclk = HSI16_VALUE;
    uint8_t clksrc = 0;

    /* Read SWS[2:0] to identify the current system clock source */
    clksrc = (uint8_t)((RCC->CFGR >> RCC_CFGR_SWS) & 0x7U);

    if (clksrc == 0U){  // 000: HSISYS
    	sysclk = HSI16_VALUE;
    }
    else if (clksrc == 1U){ // 001: HSE
    	sysclk = HSI16_VALUE;
    }
    else if (clksrc == 2U){ // 010: PLLRCLK
    	sysclk = HSI16_VALUE;
    }
    else if (clksrc == 3U){ // 011: LSI
    	sysclk = 32000U;
    }
    else if (clksrc == 4U){ // 011: LSE
    	sysclk = 32768U;
    }
    else{
    	sysclk = HSI16_VALUE;
    }

    return sysclk;
}

/* ------------------------------------------------------------
 * Public APIs
 * ------------------------------------------------------------ */

/* Return the APB1 peripheral clock frequency */
uint32_t RCC_GetPCLK1Value(void)
{
    uint32_t sysclk = 0;
    uint32_t hclk = 0;
    uint32_t pclk = 0;

    uint8_t hpre = 0;
    uint8_t ppre = 0;

    sysclk = RCC_GetSYSCLKValue();

    /* Calculate HCLK from SYSCLK and AHB prescaler. */
    hpre = (uint8_t)((RCC->CFGR >> RCC_CFGR_HPRE) & 0xFU);

    if (hpre < 8U){
        hclk = sysclk;
    }
    else{
        hclk = sysclk / AHB_PreScaler[hpre - 8U];
    }

    /* Calculate PCLK from HCLK and APB prescaler. */
    ppre = (uint8_t)((RCC->CFGR >> RCC_CFGR_PPRE) & 0x7U);

    if (ppre < 4U){
        pclk = hclk;
    }
    else{
        pclk = hclk / APB_PreScaler[ppre - 4U];
    }

    return pclk;
}

/*
 * Return the APB2 peripheral clock frequency.
 *
 * In this STM32G071 driver setup, APB1 and APB2 use the same APB
 * prescaler field, so the calculation is identical.
 */

uint32_t RCC_GetPCLK2Value(void){
	return RCC_GetPCLK1Value();
}
