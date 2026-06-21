#include "stm32g071xx_spi_driver.h"
#include <stddef.h>

/* ------------------------------------------------------------
 * Static helper functions
 * ------------------------------------------------------------ */

static void spi_txe_interrupt_handle(SPI_Handle_t *pHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pHandle);

/* ------------------------------------------------------------
 * Peripheral clock control
 * ------------------------------------------------------------ */

void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi){
    if (EnOrDi == ENABLE){
        if (pSPIx == SPI1){
            SPI1_PCLK_EN();
        }
        else if (pSPIx == SPI2){
            SPI2_PCLK_EN();
        }
        else if (pSPIx == SPI3){
            SPI3_PCLK_EN();
        }
    }
    else{
        if (pSPIx == SPI1){
            SPI1_PCLK_DI();
        }
        else if (pSPIx == SPI2){
            SPI2_PCLK_DI();
        }
        else if (pSPIx == SPI3){
            SPI3_PCLK_DI();
        }
    }
}

/* ------------------------------------------------------------
 * Initialization and de-initialization
 * ------------------------------------------------------------ */

void SPI_Init(SPI_Handle_t *pSPIHandle){
	uint32_t temp_cr1 = 0;
	uint32_t temp_cr2 = 0;

	/*
	 * 1. Enable the peripheral clock.
	 * Before writing to SPI registers, the SPI peripheral clock must be enabled.
	 */
	SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

    /*
     * 2. Configure device mode: Master or Slave.
     * CR1 MSTR bit:
     * 0 = Slave mode
     * 1 = Master mode
     */
	temp_cr1 |= (pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR);

    /*
     * 3. Configure SPI bus mode.
     *
     * Full duplex:
     * BIDIMODE = 0
     * RXONLY   = 0
     *
     * Half duplex:
     * BIDIMODE = 1
     *
     * Simplex RX only:
     * BIDIMODE = 0
     * RXONLY   = 1
     */

	if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD){
		temp_cr1 &= ~(1U << SPI_CR1_BIDIMODE);
		temp_cr1 &= ~(1U << SPI_CR1_RXONLY);
	}
	else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD){
		temp_cr1 |= 1U << SPI_CR1_BIDIMODE;
	}
	else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY){
		temp_cr1 &= ~(1U << SPI_CR1_BIDIMODE);
		temp_cr1 |= 1U << SPI_CR1_RXONLY;
	}

    /*
     * 4. Configure serial clock speed.
     * CR1 BR[2:0] bits control the SPI baud rate prescaler.
     */
	temp_cr1 |= (pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR);

    /*
     * 5. Configure CPOL.
     * CR1 CPOL bit:
     * 0 = SCK is LOW when idle
     * 1 = SCK is HIGH when idle
     */
	temp_cr1 |= (pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL);

    /*
     * 6. Configure CPHA.
     * CR1 CPHA bit:
     * 0 = data is sampled on the first clock edge
     * 1 = data is sampled on the second clock edge
     */
	temp_cr1 |= (pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA);

    /*
     * 7. Configure software slave management.
     * CR1 SSM bit:
     * 0 = Hardware slave management
     * 1 = Software slave management
     */
	temp_cr1 |= (pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM);
	/*
	 * If software slave management is enabled in master mode,
	 * set SSI to 1 so the internal NSS is high.
	 * This prevents mode fault and allows the master to generate SCK.
	 */
	if (pSPIHandle->SPIConfig.SPI_SSM == SPI_SSM_EN){
		temp_cr1 |= (1U << SPI_CR1_SSI);
	}

    /*
     * 8. Configure data size.
     * On STM32G071, data size is configured in CR2 DS[3:0], bits 11:8.
     *
     * DS = 7  means 8-bit data frame.
     * DS = 15 means 16-bit data frame.
     */
	temp_cr2 &= ~(0xFU << SPI_CR2_DS);
	temp_cr2 |= (pSPIHandle->SPIConfig.SPI_DataSize << SPI_CR2_DS);

	/*
	 * For 8-bit data size, RXNE should be set when 8 bits are received.
	 */
	if (pSPIHandle->SPIConfig.SPI_DataSize == SPI_DS_8BITS){
	    temp_cr2 |= (1U << SPI_CR2_FRXTH);
	}

    /*
     * 9. Write the configured values into the actual SPI registers.
     */
	pSPIHandle->pSPIx->CR1 = temp_cr1;
	pSPIHandle->pSPIx->CR2 = temp_cr2;

	/*
	 * Initialize SPI application states.
	 * The interrupt-based APIs use these states to know
	 * whether SPI is free or busy.
	 */
	pSPIHandle->TxState = SPI_READY;
	pSPIHandle->RxState = SPI_READY;

}

void SPI_DeInit(SPI_RegDef_t *pSPIx){
	if (pSPIx == SPI1){
		SPI1_REG_RESET();
	}
	else if (pSPIx == SPI2){
		SPI2_REG_RESET();
	}
	else if (pSPIx == SPI3){
		SPI3_REG_RESET();
	}
}

/* ------------------------------------------------------------
 * Peripheral control and status
 * ------------------------------------------------------------ */

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi){
    if (EnOrDi == ENABLE){
        pSPIx->CR1 |= (1U << SPI_CR1_SPE);
    }
    else{
        pSPIx->CR1 &= ~(1U << SPI_CR1_SPE);
    }
}

void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi){
    if (EnOrDi == ENABLE){
        pSPIx->CR2 |= (1U << SPI_CR2_SSOE);
    }
    else{
        pSPIx->CR2 &= ~(1U << SPI_CR2_SSOE);
    }
}

uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName){
	if (pSPIx->SR & FlagName){
		return FLAG_SET;
	}
	return FLAG_RESET;
}

/* ------------------------------------------------------------
 * Blocking data APIs
 * ------------------------------------------------------------ */

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len){
    uint8_t dataSize = 0;
    while (Len > 0U){
        while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);

        dataSize = (uint8_t)((pSPIx->CR2 >> SPI_CR2_DS) & 0xFU);

        if (dataSize == SPI_DS_16BITS){
            /*
             * Cast the uint8_t pointer to uint16_t pointer,
             * then dereference it to write 16 bits into DR.
             */
        	*((__vo uint16_t*)&pSPIx->DR) = *((uint16_t*)pTxBuffer);

            /*
             * 16-bit frame = 2 bytes were written.
             * Move the buffer pointer by 2 bytes and decrease Len by 2.
             */
            pTxBuffer += 2U;
            Len -= 2U;
        }
        else{
            /*
             * Default/common case: 8-bit data frame.
             * Write one byte into DR.
             */
        	*((__vo uint8_t*)&pSPIx->DR) = *pTxBuffer;

            /*
             * 8-bit frame = 1 byte was written.
             * Move the buffer pointer by 1 byte and decrease Len by 1.
             */
            pTxBuffer++;
            Len--;
        }
    }
}

void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len){
	while (Len > 0){
		while (SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);
		if (((pSPIx->CR2 >> SPI_CR2_DS) & 0xFU) == SPI_DS_8BITS){
			*pRxBuffer = *((__vo uint8_t*)&pSPIx->DR);
			pRxBuffer++;
		}
		else{
			*((uint16_t *)pRxBuffer) = *((__vo uint16_t *)&pSPIx->DR);
			pRxBuffer += 2;
			Len--;
		}
		Len--;
	}
}

/* ------------------------------------------------------------
 * Interrupt-based data APIs
 * ------------------------------------------------------------ */

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len){
	uint8_t state = pSPIHandle->TxState;

	if (state != SPI_BUSY_IN_TX){
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = Len;
		pSPIHandle->TxState = SPI_BUSY_IN_TX;

        pSPIHandle->pSPIx->CR2 |= (1U << SPI_CR2_TXEIE);
	}

	return state;
}

uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len){
	uint8_t state = pSPIHandle->RxState;

	if (state != SPI_BUSY_IN_RX){
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = Len;
		pSPIHandle->RxState = SPI_BUSY_IN_RX;

		pSPIHandle->pSPIx->CR2 |= (1U << SPI_CR2_RXNEIE);
	}

	return state;
}

/* ------------------------------------------------------------
 * IRQ configuration and handling
 * ------------------------------------------------------------ */

void SPI_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi){
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

void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority){
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED);

    /*
     * Only the upper priority bits are implemented on Cortex-M0+.
     */

	*(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFFU << (8U * iprx_section));
	*(NVIC_PR_BASE_ADDR + iprx) |= ((IRQPriority & ((1U << NO_PR_BITS_IMPLEMENTED) - 1U)) << shift_amount);
}

void SPI_IRQHandling(SPI_Handle_t *pHandle){
    uint8_t temp_TXE = 0U;
    uint8_t temp_TXEIE = 0U;
    uint8_t temp_RXNE = 0U;
    uint8_t temp_RXNEIE = 0U;
    uint8_t temp_OVR = 0U;
    uint8_t temp_ERRIE = 0U;

    temp_TXE = (uint8_t)(pHandle->pSPIx->SR & (1U << SPI_SR_TXE));
    temp_TXEIE = (uint8_t)(pHandle->pSPIx->CR2 & (1U << SPI_CR2_TXEIE));

    if (temp_TXE && temp_TXEIE){
        spi_txe_interrupt_handle(pHandle);
    }

    temp_RXNE = (uint8_t)(pHandle->pSPIx->SR & (1U << SPI_SR_RXNE));
    temp_RXNEIE = (uint8_t)(pHandle->pSPIx->CR2 & (1U << SPI_CR2_RXNEIE));

    if (temp_RXNE && temp_RXNEIE){
        spi_rxne_interrupt_handle(pHandle);
    }

    temp_OVR = (uint8_t)(pHandle->pSPIx->SR & (1U << SPI_SR_OVR));
    temp_ERRIE = (uint8_t)(pHandle->pSPIx->CR2 & (1U << SPI_CR2_ERRIE));

    if (temp_OVR && temp_ERRIE){
        spi_ovr_err_interrupt_handle(pHandle);
    }
}

/* ------------------------------------------------------------
 * Close and error handling helpers
 * ------------------------------------------------------------ */

void SPI_CloseTransmission(SPI_Handle_t *pHandle){
	pHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_TXEIE);

	pHandle->pTxBuffer = NULL;
	pHandle->TxLen = 0;
	pHandle->TxState = SPI_READY;
}

void SPI_CloseReception(SPI_Handle_t *pHandle){
    pHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_RXNEIE);

    pHandle->pRxBuffer = NULL;
    pHandle->RxLen = 0;
    pHandle->RxState = SPI_READY;
}

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx){
    uint8_t temp;

    temp = *((__vo uint8_t*)&pSPIx->DR);
    temp = pSPIx->SR;

    (void)temp;
}

static void spi_txe_interrupt_handle(SPI_Handle_t *pHandle){
    uint8_t dataSize = 0;

   dataSize = (uint8_t)((pHandle->pSPIx->CR2 >> SPI_CR2_DS) & 0xFU);

   if (dataSize == SPI_DS_16BITS){
       *((__vo uint16_t*)&pHandle->pSPIx->DR) = *((uint16_t*)pHandle->pTxBuffer);

       pHandle->pTxBuffer += 2U;
       pHandle->TxLen -= 2U;
   }
   else{
       *((__vo uint8_t*)&pHandle->pSPIx->DR) = *(pHandle->pTxBuffer);

       pHandle->pTxBuffer++;
       pHandle->TxLen--;
   }

   if (pHandle->TxLen == 0U){
	   SPI_CloseTransmission(pHandle);
	   SPI_ApplicationEventCallback(pHandle, SPI_EVENT_TX_CMPLT);
   }
}

static void spi_rxne_interrupt_handle(SPI_Handle_t *pHandle){
    uint8_t dataSize = 0;

   dataSize = (uint8_t)((pHandle->pSPIx->CR2 >> SPI_CR2_DS) & 0xFU);

   if (dataSize == SPI_DS_16BITS){
       *((uint16_t*)pHandle->pRxBuffer) = *((__vo uint16_t*)&pHandle->pSPIx->DR);

       pHandle->pRxBuffer += 2U;
       pHandle->RxLen -= 2U;
   }
   else{
       *(pHandle->pRxBuffer) = *((__vo uint8_t*)&pHandle->pSPIx->DR);

       pHandle->pRxBuffer++;
       pHandle->RxLen--;
   }

   if (pHandle->RxLen == 0U){
	   SPI_CloseReception(pHandle);
	   SPI_ApplicationEventCallback(pHandle, SPI_EVENT_RX_CMPLT);
   }
}

static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pHandle){
    if (pHandle->TxState != SPI_BUSY_IN_TX){
        SPI_ClearOVRFlag(pHandle->pSPIx);
    }

    SPI_ApplicationEventCallback(pHandle, SPI_EVENT_OVR_ERR);
}

/* ------------------------------------------------------------
 * Application callback
 * ------------------------------------------------------------ */

__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv){
    (void)pSPIHandle;
    (void)AppEv;
}
