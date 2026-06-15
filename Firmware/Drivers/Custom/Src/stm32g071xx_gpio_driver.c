#include "stm32g071xx_gpio_driver.h"

/* ------------------------------------------------------------
 * Peripheral clock control
 * ------------------------------------------------------------ */

void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnOrDi){
    if (EnOrDi == ENABLE){
        if (pGPIOx == GPIOA){
            GPIOA_PCLK_EN();
        }
        else if (pGPIOx == GPIOB){
            GPIOB_PCLK_EN();
        }
        else if (pGPIOx == GPIOC){
            GPIOC_PCLK_EN();
        }
        else if (pGPIOx == GPIOD){
            GPIOD_PCLK_EN();
        }
        else if (pGPIOx == GPIOE){
            GPIOE_PCLK_EN();
        }
        else if (pGPIOx == GPIOF){
            GPIOF_PCLK_EN();
        }
    }
    else{
        if (pGPIOx == GPIOA){
            GPIOA_PCLK_DI();
        }
        else if (pGPIOx == GPIOB){
            GPIOB_PCLK_DI();
        }
        else if (pGPIOx == GPIOC){
            GPIOC_PCLK_DI();
        }
        else if (pGPIOx == GPIOD){
            GPIOD_PCLK_DI();
        }
        else if (pGPIOx == GPIOE){
            GPIOE_PCLK_DI();
        }
        else if (pGPIOx == GPIOF){
            GPIOF_PCLK_DI();
        }
    }
}

/* ------------------------------------------------------------
 * Initialization and de-initialization
 * ------------------------------------------------------------ */

void GPIO_Init(GPIO_Handle_t *pGPIOHandle){
    uint32_t temp = 0;
    uint8_t pin = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber;

    /* Enable peripheral clock before configuring GPIO registers */
	GPIO_PeriClockControl(pGPIOHandle->pGPIOx, ENABLE);

    /*
     * 1. Configure GPIO mode.
     * MODER uses 2 bits per pin:
     * 00 = input, 01 = output, 10 = alternate function, 11 = analog.
     */
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG){
        temp = pGPIOHandle->GPIO_PinConfig.GPIO_PinMode << (2U * pin);

        pGPIOHandle->pGPIOx->MODER &= ~(0x3U << (2U * pin));
        pGPIOHandle->pGPIOx->MODER |= temp;
    }
    else{
        /*
         * Interrupt mode configuration.
         * Configure EXTI trigger registers and map GPIO port to EXTI line.
         */

    	pGPIOHandle->pGPIOx->MODER &= ~(0x3U << (2U * pin));

    	if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT){
    	    EXTI->FTSR1 |=  (1U << pin);
    	    EXTI->RTSR1 &= ~(1U << pin);
    	}
    	else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT){
    	    EXTI->RTSR1 |=  (1U << pin);
    	    EXTI->FTSR1 &= ~(1U << pin);
    	}
    	else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT){
    	    EXTI->FTSR1 |= (1U << pin);
    	    EXTI->RTSR1 |= (1U << pin);
    	}

        /* Select which GPIO port is connected to this EXTI line */
    	uint8_t exti_index = pin / 4;
    	uint8_t exti_position = pin % 4;

    	SYSCFG_PCLK_EN();

    	EXTI->EXTICR[exti_index] &= ~(0xFFFF << (exti_position * 4));
    	EXTI->EXTICR[exti_index] |= (GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx) << (exti_position * 4));

    	EXTI->IMR1 |= (1U << pin);
    }

    /*
     * 2. Configure GPIO output speed.
     * OSPEEDR uses 2 bits per pin.
     */
    temp = pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed << (2U * pin);

    /* Clear only the speed bits of the selected pin. */
    pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x3U << (2U * pin));

    /* Set the new speed bits for the selected pin. */
    pGPIOHandle->pGPIOx->OSPEEDR |= temp;

    /*
     * 3. Configure pull-up / pull-down.
     * PUPDR uses 2 bits per pin:
     * 00 = no pull-up/pull-down, 01 = pull-up, 10 = pull-down.
     */
    temp = pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl << (2U * pin);

    /* Clear only the pull-up/pull-down bits of the selected pin. */
    pGPIOHandle->pGPIOx->PUPDR &= ~(0x3U << (2U * pin));

    /* Set the new pull-up/pull-down configuration. */
    pGPIOHandle->pGPIOx->PUPDR |= temp;

    /*
     * 4. Configure output type.
     * OTYPER uses 1 bit per pin:
     * 0 = push-pull, 1 = open-drain.
     */
    temp = pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType << pin;

    /* Clear only the output type bit of the selected pin. */
    pGPIOHandle->pGPIOx->OTYPER &= ~(0x1U << pin);

    /* Set the new output type bit. */
    pGPIOHandle->pGPIOx->OTYPER |= temp;

    /*
     * 5. Configure alternate function, only if the pin is in alternate function mode.
     *
     * AFR[0] = AFRL for pins 0..7
     * AFR[1] = AFRH for pins 8..15
     *
     * Each pin gets 4 bits in AFR.
     */
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN){
        uint8_t afr_index = pin / 8U;   /* 0 for pins 0-7, 1 for pins 8-15 */
        uint8_t afr_pos = pin % 8U;     /* Pin position inside AFRL/AFRH */

        /* Clear the 4 alternate-function bits of the selected pin. */
        pGPIOHandle->pGPIOx->AFR[afr_index] &= ~(0xFU << (4U * afr_pos));

        /* Set the selected alternate function value. */
        pGPIOHandle->pGPIOx->AFR[afr_index] |=
            (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << (4U * afr_pos));
    }
}

void GPIO_DeInit(GPIO_RegDef_t *pGPIOx){
    if (pGPIOx == GPIOA){
        GPIOA_REG_RESET();
    }
    else if (pGPIOx == GPIOB){
        GPIOB_REG_RESET();
    }
    else if (pGPIOx == GPIOC){
        GPIOC_REG_RESET();
    }
    else if (pGPIOx == GPIOD){
        GPIOD_REG_RESET();
    }
    else if (pGPIOx == GPIOE){
        GPIOE_REG_RESET();
    }
    else if (pGPIOx == GPIOF){
        GPIOF_REG_RESET();
    }
}

/* ------------------------------------------------------------
 * Input APIs
 * ------------------------------------------------------------ */

uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber){
	return (uint8_t)((pGPIOx->IDR >> PinNumber) & 0x1U);
}

uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx){
	return (uint16_t)(pGPIOx->IDR & 0xFFFFU);
}

/* ------------------------------------------------------------
 * Output APIs
 * ------------------------------------------------------------ */

void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value){
    if (Value == GPIO_PIN_SET){
        pGPIOx->ODR |= (1U << PinNumber);
    }
    else{
        pGPIOx->ODR &= ~(1U << PinNumber);
    }
}

void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value){
    pGPIOx->ODR = Value;
}

void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber){
    pGPIOx->ODR ^= (1U << PinNumber);
}

/* ------------------------------------------------------------
 * IRQ configuration and handling
 * ------------------------------------------------------------ */

void GPIO_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi){
	if (EnOrDi == ENABLE){
		if (IRQNumber < 32){
			*NVIC_ISER0 |= (1U << IRQNumber);
		}
		else if (IRQNumber < 64){
			*NVIC_ISER1 |= (1U << (IRQNumber % 32));
		}
		else if (IRQNumber < 96){
			*NVIC_ISER2 |= (1U << (IRQNumber % 64));
		}
	}
	else{
		if (IRQNumber < 32){
			*NVIC_ICER0 |= (1U << IRQNumber);
		}
		else if (IRQNumber < 64){
			*NVIC_ICER1 |= (1U << (IRQNumber % 32));
		}
		else if (IRQNumber < 96){
			*NVIC_ICER2 |= (1U << (IRQNumber % 64));
		}
	}
}

void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority){
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED);

	/* Only the upper priority bits are implemented on Cortex-M0+ */
	*(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFFU << (8U * iprx_section));
	*(NVIC_PR_BASE_ADDR + iprx) |= ((IRQPriority & ((1U << NO_PR_BITS_IMPLEMENTED) - 1U)) << shift_amount);
}

void GPIO_IRQHandling(uint8_t PinNumber){
   if (EXTI->FPR1 & (1U << PinNumber)){
	   EXTI->FPR1 |= (1U << PinNumber);
   }
   if (EXTI->RPR1 & (1U << PinNumber)){
	   EXTI->RPR1 |= (1U << PinNumber);
   }
}
