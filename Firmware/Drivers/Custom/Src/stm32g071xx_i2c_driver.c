#include "stm32g071xx_i2c_driver.h"
#include <stddef.h>

/* ------------------------------------------------------------
 * Static helper functions
 * ------------------------------------------------------------ */

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle);
void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle);

/* ------------------------------------------------------------
 * Peripheral clock control
 * ------------------------------------------------------------ */

void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi){
	if (EnOrDi == ENABLE){
		if (pI2Cx == I2C1){
			I2C1_PCLK_EN();
		}
		else if (pI2Cx == I2C2){
			I2C2_PCLK_EN();
		}
	}
	else {
		if (pI2Cx == I2C1){
			I2C1_PCLK_DI();
		}
		else if (pI2Cx == I2C2){
			I2C2_PCLK_DI();
		}
	}
}

/* ------------------------------------------------------------
 * Initialization and de-initialization
 * ------------------------------------------------------------ */

void I2C_Init(I2C_Handle_t *pI2CHandle){
	uint32_t tempreg = 0;

	I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

	I2C_PeripheralControl(pI2CHandle->pI2Cx, DISABLE);

	if (pI2CHandle->I2CConfig.I2C_SCLSpeed == I2C_SCL_SPEED_SM){
		pI2CHandle->pI2Cx->TIMINGR = 0x00303D5BU;
	}
	tempreg = 0;
	tempreg |= ((uint32_t)(pI2CHandle->I2CConfig.I2C_DeviceAddress & 0x7FU) << 1);
	tempreg &= ~(1U << I2C_OAR1_OA1MODE);
	tempreg |= (1U << I2C_OAR1_OA1EN);
	pI2CHandle->pI2Cx->OAR1 = tempreg;


	pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_NOSTRETCH);
	I2C_PeripheralControl(pI2CHandle->pI2Cx, ENABLE);
	pI2CHandle->TxRxState = I2C_READY;
}

void I2C_DeInit(I2C_RegDef_t *pI2Cx){
	if (pI2Cx == I2C1){
		I2C1_REG_RESET();
	}
	else if (pI2Cx == I2C2){
		I2C2_REG_RESET();
	}
}

/* ------------------------------------------------------------
 * Peripheral control and status
 * ------------------------------------------------------------ */

void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi){
	if (EnOrDi == ENABLE){
		pI2Cx->CR1 |= (1U << I2C_CR1_PE);
	}
	else{
		pI2Cx->CR1 &= ~(1U << I2C_CR1_PE);
	}
}

uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName){
	if (pI2Cx->ISR & FlagName){
		return FLAG_SET;
	}
	else{
		return FLAG_RESET;
	}
}

/* ------------------------------------------------------------
 * Blocking master APIs
 * ------------------------------------------------------------ */

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr){
    uint32_t tempreg = 0;

    while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_BUSY_FLAG) == FLAG_SET);

    tempreg |= ((((uint32_t)SlaveAddr & 0x7FU) << 1) << I2C_CR2_SADD);
    tempreg &= ~(1U << I2C_CR2_RD_WRN);
    tempreg |= ((Len & 0xFFU) << I2C_CR2_NBYTES);
    tempreg &= ~(1U << I2C_CR2_AUTOEND);
    tempreg |= (1U << I2C_CR2_START);

    pI2CHandle->pI2Cx->CR2 = tempreg;

    while (Len > 0){
        while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_TXIS_FLAG) == FLAG_RESET){
        	if (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_NACKF_FLAG) == FLAG_SET){
        	    pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_NACKCF);
        	    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_STOP);

        	    while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_STOPF_FLAG) == FLAG_RESET);

        	    pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_STOPCF);
        	    pI2CHandle->pI2Cx->CR2 = 0;
        	    return;
        	}
        }

        pI2CHandle->pI2Cx->TXDR = *pTxBuffer;
        pTxBuffer++;
        Len--;
    }

    while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_TC_FLAG) == FLAG_RESET){
    	if (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_NACKF_FLAG) == FLAG_SET){
    	    pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_NACKCF);
    	    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_STOP);

    	    while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_STOPF_FLAG) == FLAG_RESET);

    	    pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_STOPCF);
    	    pI2CHandle->pI2Cx->CR2 = 0;

    	    return;
    	}
    }

    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_STOP);

    while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_STOPF_FLAG) == FLAG_RESET);

    pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_STOPCF);
    pI2CHandle->pI2Cx->CR2 = 0;
}


void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr){
	uint32_t tempreg = 0;

    if (Len == 0U || Len > 255U){
        return;
    }

	while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_BUSY_FLAG) == FLAG_SET);

	tempreg |= ((((uint32_t)SlaveAddr & 0x7FU) << 1) << I2C_CR2_SADD);
	tempreg |= (1U << I2C_CR2_RD_WRN);
	tempreg |= ((Len & 0xFFU) << I2C_CR2_NBYTES);
	tempreg |= (1U << I2C_CR2_AUTOEND);
	tempreg |= (1U << I2C_CR2_START);

	pI2CHandle->pI2Cx->CR2 = tempreg;

	while (Len > 0){
		while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_RXNE_FLAG) == FLAG_RESET){
			if (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_NACKF_FLAG) == FLAG_SET){
				pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_NACKCF);
				while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_STOPF_FLAG) == FLAG_RESET);
				pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_STOPCF);
				pI2CHandle->pI2Cx->CR2 = 0;
				return;
			}
		}

		*pRxBuffer = (uint8_t)pI2CHandle->pI2Cx->RXDR;
		pRxBuffer++;
		Len--;
	}

	while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_STOPF_FLAG) == FLAG_RESET);

	pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_STOPCF);
	pI2CHandle->pI2Cx->CR2 = 0;
}

/* ------------------------------------------------------------
 * Slave APIs
 * ------------------------------------------------------------ */

void I2C_SlaveSendData(I2C_RegDef_t *pI2Cx, uint8_t data){
	pI2Cx->TXDR = data;
}

uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2Cx){
	return (uint8_t)pI2Cx->RXDR;
}

void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi){
    if (EnOrDi == ENABLE){
        pI2Cx->CR1 |= (1U << I2C_CR1_ADDRIE);
        pI2Cx->CR1 |= (1U << I2C_CR1_RXIE);
        pI2Cx->CR1 |= (1U << I2C_CR1_TXIE);
        pI2Cx->CR1 |= (1U << I2C_CR1_STOPIE);
        pI2Cx->CR1 |= (1U << I2C_CR1_NACKIE);
        pI2Cx->CR1 |= (1U << I2C_CR1_ERRIE);
    }
    else{
        pI2Cx->CR1 &= ~(1U << I2C_CR1_ADDRIE);
        pI2Cx->CR1 &= ~(1U << I2C_CR1_RXIE);
        pI2Cx->CR1 &= ~(1U << I2C_CR1_TXIE);
        pI2Cx->CR1 &= ~(1U << I2C_CR1_STOPIE);
        pI2Cx->CR1 &= ~(1U << I2C_CR1_NACKIE);
        pI2Cx->CR1 &= ~(1U << I2C_CR1_ERRIE);
    }
}

/* ------------------------------------------------------------
 * Interrupt-based master APIs
 * ------------------------------------------------------------ */

uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr){
	uint8_t state = pI2CHandle->TxRxState;
	uint32_t tempreg = 0;

	if (state == I2C_READY){
		pI2CHandle->pTxBuffer = pTxBuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;

		tempreg |= ((((uint32_t)SlaveAddr & 0x7FU) << 1) << I2C_CR2_SADD);
		tempreg &= ~(1U << I2C_CR2_RD_WRN);
		tempreg |= ((Len & 0xFFU) << I2C_CR2_NBYTES);

		if (Sr == I2C_ENABLE_SR){
			tempreg &= ~(1U << I2C_CR2_AUTOEND);
		}
		else{
			tempreg |= (1U << I2C_CR2_AUTOEND);
		}

		pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_TXIE);
		pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_TCIE);
		pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_ERRIE);
		pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_NACKIE);
		pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_STOPIE);

		tempreg |= (1U << I2C_CR2_START);

		pI2CHandle->pI2Cx->CR2 = tempreg;
	}

	return state;
}

uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr){
	uint8_t state = pI2CHandle->TxRxState;
	uint32_t tempreg = 0;

	if (state == I2C_READY){
			pI2CHandle->pRxBuffer = pRxBuffer;
			pI2CHandle->RxLen = Len;
			pI2CHandle->RxSize = Len;
			pI2CHandle->DevAddr = SlaveAddr;
			pI2CHandle->Sr = Sr;
			pI2CHandle->TxRxState = I2C_BUSY_IN_RX;

			tempreg |= ((((uint32_t)SlaveAddr & 0x7FU) << 1) << I2C_CR2_SADD);
			tempreg |= (1U << I2C_CR2_RD_WRN);
			tempreg |= ((Len & 0xFFU) << I2C_CR2_NBYTES);

			if (Sr == I2C_DISABLE_SR){
				tempreg |= (1U << I2C_CR2_AUTOEND);
			}
			else{
				tempreg &= ~(1U << I2C_CR2_AUTOEND);
			}

			pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_RXIE);
			pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_TCIE);
			pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_NACKIE);
			pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_ERRIE);
			pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_STOPIE);

			tempreg |= (1U << I2C_CR2_START);

			pI2CHandle->pI2Cx->CR2 = tempreg;
		}

	return state;
}

/* ------------------------------------------------------------
 * IRQ configuration and handling
 * ------------------------------------------------------------ */

void I2C_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi){
    (void)IRQPriority;

    if (EnOrDi == ENABLE){
        if (IRQNumber < 32){
            *NVIC_ISER0 |= (1U << IRQNumber);
        }
        else if (IRQNumber < 64){
            *NVIC_ISER1 |= (1U << (IRQNumber % 32));
        }
        else if (IRQNumber < 96){
            *NVIC_ISER2 |= (1U << (IRQNumber % 32));
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
            *NVIC_ICER2 |= (1U << (IRQNumber % 32));
        }
    }
}


void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority){
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED);

	*(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFFU << (8U * iprx_section));
	*(NVIC_PR_BASE_ADDR + iprx) |= ((IRQPriority & ((1U << NO_PR_BITS_IMPLEMENTED) - 1U)) << shift_amount);
}

void I2C_IRQHandling(I2C_Handle_t *pI2CHandle){
	if ((pI2CHandle->pI2Cx->ISR & I2C_NACKF_FLAG) && (pI2CHandle->pI2Cx->CR1 &(1U << I2C_CR1_NACKIE))){
		pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_NACKCF);
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_STOP);
			I2C_CloseSendData(pI2CHandle);
		}
		else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_STOP);
			I2C_CloseReceiveData(pI2CHandle);
		}
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_AF);
	}

	if ((pI2CHandle->pI2Cx->ISR & (1U << I2C_ISR_ADDR)) && (pI2CHandle->pI2Cx->CR1 & (1U << I2C_CR1_ADDRIE))){
		pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_ADDRCF);
	}

	if ((pI2CHandle->pI2Cx->ISR & I2C_RXNE_FLAG) && (pI2CHandle->pI2Cx->CR1 &(1U << I2C_CR1_RXIE))){
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			if (pI2CHandle->RxLen > 0){
				*(pI2CHandle->pRxBuffer) = pI2CHandle->pI2Cx->RXDR;
				pI2CHandle->pRxBuffer++;
				pI2CHandle->RxLen--;
			}
		}
		else{
			I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);
		}
	}

	if ((pI2CHandle->pI2Cx->ISR & I2C_TXIS_FLAG) && (pI2CHandle->pI2Cx->CR1 & (1U << I2C_CR1_TXIE))){
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			if (pI2CHandle->TxLen > 0){
				pI2CHandle->pI2Cx->TXDR = *(pI2CHandle->pTxBuffer);
				pI2CHandle->pTxBuffer++;
				pI2CHandle->TxLen--;
			}
		}
		else{
			I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
		}
	}

	if ((pI2CHandle->pI2Cx->ISR & I2C_TC_FLAG) && (pI2CHandle->pI2Cx->CR1 &(1U << I2C_CR1_TCIE))){
		if (pI2CHandle->Sr == I2C_DISABLE_SR){
			pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_STOP);
		}
	}

	if ((pI2CHandle->pI2Cx->ISR & I2C_STOPF_FLAG) && (pI2CHandle->pI2Cx->CR1 &(1U << I2C_CR1_STOPIE))){
		pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_STOPCF);
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			I2C_CloseSendData(pI2CHandle);
			I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
		}
		else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			I2C_CloseReceiveData(pI2CHandle);
			I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
		}
		else{
			I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);
		}
	}

	if ((pI2CHandle->pI2Cx->ISR & I2C_BERR_FLAG) && (pI2CHandle->pI2Cx->CR1 &(1U << I2C_CR1_ERRIE))){
		pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_BERRCF);
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			I2C_CloseSendData(pI2CHandle);
		}
		else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			I2C_CloseReceiveData(pI2CHandle);
		}
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_BERR);
	}

	if ((pI2CHandle->pI2Cx->ISR & I2C_ARLO_FLAG) && (pI2CHandle->pI2Cx->CR1 &(1U << I2C_CR1_ERRIE))){
		pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_ARLOCF);
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			I2C_CloseSendData(pI2CHandle);
		}
		else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			I2C_CloseReceiveData(pI2CHandle);
		}
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_ARLO);
	}

	if ((pI2CHandle->pI2Cx->ISR & I2C_OVR_FLAG) && (pI2CHandle->pI2Cx->CR1 &(1U << I2C_CR1_ERRIE))){
		pI2CHandle->pI2Cx->ICR = (1U << I2C_ICR_OVRCF);
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			I2C_CloseSendData(pI2CHandle);
		}
		else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			I2C_CloseReceiveData(pI2CHandle);
		}
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);
	}
}

/* ------------------------------------------------------------
 * Close helper functions
 * ------------------------------------------------------------ */

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle){
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_TXIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_TCIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_STOPIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_NACKIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_ERRIE);

    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->pTxBuffer = NULL;
    pI2CHandle->TxLen = 0;
}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle){
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_RXIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_TCIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_STOPIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_NACKIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_ERRIE);

    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->pRxBuffer = NULL;
    pI2CHandle->RxLen = 0;
    pI2CHandle->RxSize = 0;
}

/* ------------------------------------------------------------
 * Application callback
 * ------------------------------------------------------------ */

__weak void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv){
    (void)pI2CHandle;
    (void)AppEv;
}
