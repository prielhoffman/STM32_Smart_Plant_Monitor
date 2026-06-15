#include "stm32g071xx_usart_driver.h"
#include "stm32g071xx_rcc_driver.h"

#include <stddef.h>

/* ------------------------------------------------------------
 * Static helper functions
 * ------------------------------------------------------------ */

static void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate);

/* ------------------------------------------------------------
 * Peripheral clock control
 * ------------------------------------------------------------ */

void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi){
	if (EnOrDi == ENABLE){
		if (pUSARTx == USART1){
			USART1_PCLK_EN();
		}
		else if (pUSARTx == USART2){
			USART2_PCLK_EN();
		}
		else if (pUSARTx == USART3){
			USART3_PCLK_EN();
		}
		else if (pUSARTx == USART4){
			USART4_PCLK_EN();
		}
		else if (pUSARTx == USART5){
			USART5_PCLK_EN();
		}
		else if (pUSARTx == USART6){
			USART6_PCLK_EN();
		}
	}
	else{
		if (pUSARTx == USART1){
			USART1_PCLK_DI();
		}
		else if (pUSARTx == USART2){
			USART2_PCLK_DI();
		}
		else if (pUSARTx == USART3){
			USART3_PCLK_DI();
		}
		else if (pUSARTx == USART4){
			USART4_PCLK_DI();
		}
		else if (pUSARTx == USART5){
			USART5_PCLK_DI();
		}
		else if (pUSARTx == USART6){
			USART6_PCLK_DI();
		}
	}
}

/* ------------------------------------------------------------
 * Initialization and de-initialization
 * ------------------------------------------------------------ */

void USART_Init(USART_Handle_t *pHandle){
	uint32_t tempreg = 0;

	USART_PeriClockControl(pHandle->pUSARTx, ENABLE);
	USART_PeripheralControl(pHandle->pUSARTx, DISABLE);

	tempreg |= (pHandle->USARTConfig.USART_Mode << USART_CR1_RE);

	if (pHandle->USARTConfig.USART_WordLength == USART_WORDLEN_9BITS){
		tempreg |= 1U << USART_CR1_M0;
	}
	else if (pHandle->USARTConfig.USART_WordLength == USART_WORDLEN_7BITS){
		tempreg |= 1U << USART_CR1_M1;
	}

	tempreg &= ~(0x3U << USART_CR1_PS);
	tempreg |= ((uint32_t)pHandle->USARTConfig.USART_ParityControl << USART_CR1_PS);
	tempreg &= ~(1U << USART_CR1_OVER8);

	pHandle->pUSARTx->CR1 = tempreg;

	USART_SetBaudRate(pHandle->pUSARTx, pHandle->USARTConfig.USART_Baud);

	tempreg = 0;
	tempreg |= (pHandle->USARTConfig.USART_NoOfStopBits << USART_CR2_STOP);
	pHandle->pUSARTx->CR2 = tempreg;

	tempreg = 0;
	tempreg |= ((uint32_t)pHandle->USARTConfig.USART_HWFlowControl << USART_CR3_RTSE);

	pHandle->pUSARTx->CR3 = tempreg;
	pHandle->TxState = USART_READY;
	pHandle->RxState = USART_READY;

	USART_PeripheralControl(pHandle->pUSARTx, ENABLE);
}


void USART_DeInit(USART_RegDef_t *pUSARTx){
	if (pUSARTx == USART1){
		USART1_REG_RESET();
	}
	else if (pUSARTx == USART2){
		USART2_REG_RESET();
	}
	else if (pUSARTx == USART3){
		USART3_REG_RESET();
	}
	else if (pUSARTx == USART4){
		USART4_REG_RESET();
	}
	else if (pUSARTx == USART5){
		USART5_REG_RESET();
	}
	else if (pUSARTx == USART6){
		USART6_REG_RESET();
	}
}

/* ------------------------------------------------------------
 * Peripheral control and status
 * ------------------------------------------------------------ */

void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi){
	if (EnOrDi == ENABLE){
		pUSARTx->CR1 |= (1U << USART_CR1_UE);
	}
	else{
		pUSARTx->CR1 &= ~(1U << USART_CR1_UE);
	}
}

uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint32_t FlagName){
	if (pUSARTx->ISR & FlagName){
		return FLAG_SET;
	}
	else{
		return FLAG_RESET;
	}
}

void USART_ClearFlag(USART_RegDef_t *pUSARTx, uint16_t StatusFlagName){
	if (StatusFlagName == USART_TC_FLAG){
        pUSARTx->ICR |= (1U << USART_ICR_TCCF);
    }
    else if (StatusFlagName == USART_ORE_FLAG){
        pUSARTx->ICR |= (1U << USART_ICR_ORECF);
    }
    else if (StatusFlagName == USART_IDLE_FLAG){
        pUSARTx->ICR |= (1U << USART_ICR_IDLECF);
    }
    else if (StatusFlagName == USART_NE_FLAG){
        pUSARTx->ICR |= (1U << USART_ICR_NECF);
    }
    else if (StatusFlagName == USART_FE_FLAG){
        pUSARTx->ICR |= (1U << USART_ICR_FECF);
    }
    else if (StatusFlagName == USART_PE_FLAG){
        pUSARTx->ICR |= (1U << USART_ICR_PECF);
    }
}

void USART_ClearOREFlag(USART_RegDef_t *pUSARTx){
    USART_ClearFlag(pUSARTx, USART_ORE_FLAG);
}

/* ------------------------------------------------------------
 * Baud-rate configuration
 * ------------------------------------------------------------ */

static void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate){
    uint32_t pclk = 0;
    uint32_t usartdiv = 0;

    if ((pUSARTx == USART1) || (pUSARTx == USART6)){
    	pclk = RCC_GetPCLK2Value();
    }
    else{
    	pclk = RCC_GetPCLK1Value();
    }
    /*
     * For now we assume:
     * - USART peripheral clock = 16 MHz
     * - OVER8 = 0, oversampling by 16
     * - PRESC = 0, no USART prescaler
     *
     * In this simple case:
     * BRR = peripheral clock / baud rate
     */
    usartdiv = (pclk + (BaudRate / 2U)) / BaudRate;

    pUSARTx->BRR = usartdiv;
}

/* ------------------------------------------------------------
 * Blocking data APIs
 * ------------------------------------------------------------ */

void USART_SendData(USART_Handle_t *pHandle, uint8_t *pTxBuffer, uint32_t Len){
	uint16_t data = 0;

	while (Len > 0){
		while (USART_GetFlagStatus(pHandle->pUSARTx, USART_TXE_FLAG) == FLAG_RESET);
		if (pHandle->USARTConfig.USART_WordLength == USART_WORDLEN_9BITS){
			if (pHandle->USARTConfig.USART_ParityControl == USART_PARITY_DISABLE){
				if (Len < 2U){
					break;
				}
				data =  *((uint16_t*)pTxBuffer);
				pHandle->pUSARTx->TDR = (data & 0x1FFU);
				pTxBuffer += 2U;
				Len -= 2U;
			}
			else{
				pHandle->pUSARTx->TDR = (*pTxBuffer & 0xFFU);
				pTxBuffer++;
				Len--;
			}
		}
		else{
			pHandle->pUSARTx->TDR = (*pTxBuffer & 0xFFU);
			pTxBuffer++;
			Len--;
		}
	}
	while (USART_GetFlagStatus(pHandle->pUSARTx, USART_TC_FLAG) == FLAG_RESET);
}

void USART_ReceiveData(USART_Handle_t *pHandle, uint8_t *pRxBuffer, uint32_t Len){
	uint16_t data = 0;

	while (Len > 0){
		while (USART_GetFlagStatus(pHandle->pUSARTx, USART_RXNE_FLAG) == FLAG_RESET);

		if (pHandle->USARTConfig.USART_WordLength == USART_WORDLEN_9BITS){
			data = (uint16_t)(pHandle->pUSARTx->RDR);
			if (pHandle->USARTConfig.USART_ParityControl == USART_PARITY_DISABLE){
				if (Len < 2U){
					break;
				}
				*((uint16_t*)pRxBuffer) = (data & 0x01FFU);
				pRxBuffer += 2U;
				Len -= 2U;
			}
			else{
				*pRxBuffer =(uint8_t)(data & 0xFFU);
				pRxBuffer++;
				Len--;
			}
		}
		else if (pHandle->USARTConfig.USART_WordLength == USART_WORDLEN_8BITS){
			data = (uint16_t)(pHandle->pUSARTx->RDR);
			if (pHandle->USARTConfig.USART_ParityControl == USART_PARITY_DISABLE){
				*pRxBuffer =(uint8_t)(data & 0xFFU);
			}
			else{
				*pRxBuffer =(uint8_t)(data & 0x7FU);
			}
			pRxBuffer++;
			Len--;
		}
		else{
			data = (uint16_t)(pHandle->pUSARTx->RDR);
			if (pHandle->USARTConfig.USART_ParityControl == USART_PARITY_DISABLE){
				*pRxBuffer =(uint8_t)(data & 0x7FU);
			}
			else{
				*pRxBuffer =(uint8_t)(data & 0x3FU);
			}
			pRxBuffer++;
			Len--;
		}
	}
}

/* ------------------------------------------------------------
 * Interrupt-based data APIs
 * ------------------------------------------------------------ */

uint8_t USART_SendDataIT(USART_Handle_t *pHandle, uint8_t *pTxBuffer, uint32_t Len){
	uint8_t state = pHandle->TxState;

	if (state != USART_BUSY_IN_TX){
		pHandle->pTxBuffer = pTxBuffer;
		pHandle->TxLen = Len;
		pHandle->TxState = USART_BUSY_IN_TX;
		pHandle->pUSARTx->CR1 |= (1U << USART_CR1_TXEIE);
		pHandle->pUSARTx->CR1 |= (1U << USART_CR1_TCIE);
	}

	return state;
}

uint8_t USART_ReceiveDataIT(USART_Handle_t *pHandle, uint8_t *pRxBuffer, uint32_t Len){
	uint8_t state = pHandle->RxState;

	if (state != USART_BUSY_IN_RX){
		pHandle->pRxBuffer = pRxBuffer;
		pHandle->RxLen = Len;
		pHandle->RxState = USART_BUSY_IN_RX;
		pHandle->pUSARTx->CR1 |= (1U << USART_CR1_RXNEIE);
	}

	return state;
}

/* ------------------------------------------------------------
 * IRQ configuration and handling
 * ------------------------------------------------------------ */

void USART_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi){
    if (EnOrDi == ENABLE){
        if (IRQNumber <= 31U){
            *NVIC_ISER0 |= (1U << IRQNumber);
        }
        else if (IRQNumber <= 63U){
            *NVIC_ISER1 |= (1U << (IRQNumber % 32U));
        }
        else if (IRQNumber <= 95U){
            *NVIC_ISER2 |= (1U << (IRQNumber % 64U));
        }

        USART_IRQPriorityConfig(IRQNumber, IRQPriority);
    }
    else{
        if (IRQNumber <= 31U){
            *NVIC_ICER0 |= (1U << IRQNumber);
        }
        else if (IRQNumber <= 63U){
            *NVIC_ICER1 |= (1U << (IRQNumber % 32U));
        }
        else if (IRQNumber <= 95U){
            *NVIC_ICER2 |= (1U << (IRQNumber % 64U));
        }
    }
}

void USART_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority){
    uint8_t iprx = IRQNumber / 4U;
    uint8_t iprx_section = IRQNumber % 4U;

    uint8_t shift_amount = (uint8_t)((8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED));

    *(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFFU << (8U * iprx_section));
    *(NVIC_PR_BASE_ADDR + iprx) |= ((uint32_t)IRQPriority << shift_amount);
}

void USART_IRQHandling(USART_Handle_t *pHandle){
	if ((pHandle->pUSARTx->ISR & USART_TXE_FLAG) && (pHandle->pUSARTx->CR1 & (1U << USART_CR1_TXEIE))){
		if (pHandle->TxState == USART_BUSY_IN_TX){
			if (pHandle->TxLen > 0){
				if (pHandle->USARTConfig.USART_WordLength == USART_WORDLEN_9BITS){
					uint16_t *pdata = (uint16_t*)pHandle->pTxBuffer;
					pHandle->pUSARTx->TDR = (*pdata & 0x01FFU);
					if (pHandle->USARTConfig.USART_ParityControl == USART_PARITY_DISABLE){
						pHandle->pTxBuffer += 2U;
						pHandle->TxLen -= 2U;
					}
					else{
						pHandle->pTxBuffer++;
						pHandle->TxLen--;
					}
				}
				else{
					pHandle->pUSARTx->TDR = (*pHandle->pTxBuffer & 0xFFU);
					pHandle->pTxBuffer++;
					pHandle->TxLen--;
				}
			}
			if (pHandle->TxLen == 0){
				pHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_TXEIE);
			}
		}
	}

	if ((pHandle->pUSARTx->ISR & USART_TC_FLAG) && (pHandle->pUSARTx->CR1 & (1U << USART_CR1_TCIE))){
		if (pHandle->TxState == USART_BUSY_IN_TX){
			if (pHandle->TxLen == 0){
				USART_ClearFlag(pHandle->pUSARTx, USART_TC_FLAG);
				USART_CloseTransmission(pHandle);
				USART_ApplicationEventCallback(pHandle, USART_EVENT_TX_CMPLT);
			}
		}
	}

	if ((pHandle->pUSARTx->ISR & USART_RXNE_FLAG) && (pHandle->pUSARTx->CR1 & (1U << USART_CR1_RXNEIE))){
	    if (pHandle->RxState == USART_BUSY_IN_RX){
	        if (pHandle->RxLen > 0U){
	            uint16_t data = (uint16_t)pHandle->pUSARTx->RDR;

	            if (pHandle->USARTConfig.USART_WordLength == USART_WORDLEN_9BITS){
	                if (pHandle->USARTConfig.USART_ParityControl == USART_PARITY_DISABLE){
	                    *((uint16_t*)pHandle->pRxBuffer) = (data & 0x01FFU);
	                    pHandle->pRxBuffer += 2U;
	                    pHandle->RxLen -= 2U;
	                }
	                else{
	                    *(pHandle->pRxBuffer) = (uint8_t)(data & 0xFFU);
	                    pHandle->pRxBuffer++;
	                    pHandle->RxLen--;
	                }
	            }
	            else if (pHandle->USARTConfig.USART_WordLength == USART_WORDLEN_8BITS){
	                if (pHandle->USARTConfig.USART_ParityControl == USART_PARITY_DISABLE){
	                    *(pHandle->pRxBuffer) = (uint8_t)(data & 0xFFU);
	                }
	                else{
	                    *(pHandle->pRxBuffer) = (uint8_t)(data & 0x7FU);
	                }

	                pHandle->pRxBuffer++;
	                pHandle->RxLen--;
	            }
	            else{
	                if (pHandle->USARTConfig.USART_ParityControl == USART_PARITY_DISABLE){
	                    *(pHandle->pRxBuffer) = (uint8_t)(data & 0x7FU);
	                }
	                else{
	                    *(pHandle->pRxBuffer) = (uint8_t)(data & 0x3FU);
	                }

	                pHandle->pRxBuffer++;
	                pHandle->RxLen--;
	            }
	        }

	        if (pHandle->RxLen == 0U){
	            USART_CloseReception(pHandle);
	            USART_ApplicationEventCallback(pHandle, USART_EVENT_RX_CMPLT);
	        }
	    }
	}

	if ((pHandle->pUSARTx->ISR & USART_ORE_FLAG) && (pHandle->pUSARTx->CR1 & (1U << USART_CR1_RXNEIE)))
	{
	    USART_ClearFlag(pHandle->pUSARTx, USART_ORE_FLAG);
	    USART_ApplicationEventCallback(pHandle, USART_EVENT_ORE);
	}
}

/* ------------------------------------------------------------
 * Close helper functions
 * ------------------------------------------------------------ */

void USART_CloseTransmission(USART_Handle_t *pHandle){
	pHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_TXEIE);
	pHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_TCIE);
	pHandle->pTxBuffer = NULL;
	pHandle->TxLen = 0;
	pHandle->TxState = USART_READY;
}

void USART_CloseReception(USART_Handle_t *pHandle){
	pHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_RXNEIE);
	pHandle->pRxBuffer = NULL;
	pHandle->RxLen = 0;
	pHandle->RxState = USART_READY;
}

/* ------------------------------------------------------------
 * Application callback
 * ------------------------------------------------------------ */

__attribute__((weak)) void USART_ApplicationEventCallback(USART_Handle_t *pHandle, uint8_t AppEv)
{
    /*
     * This is a weak implementation.
     * The application can override this function in main.c.
     */
}


