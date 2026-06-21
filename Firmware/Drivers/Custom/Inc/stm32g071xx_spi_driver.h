#ifndef INC_STM32G071XX_SPI_DRIVER_H_
#define INC_STM32G071XX_SPI_DRIVER_H_

#include "stm32g071xx.h"

/* ------------------------------------------------------------
 * Configuration structures
 * ------------------------------------------------------------ */

typedef struct
{
    uint8_t SPI_DeviceMode; /* Master or slave mode */
    uint8_t SPI_BusConfig;  /* Full-duplex, half-duplex, or simplex RX-only */
    uint8_t SPI_SclkSpeed;  /* SPI baud rate prescaler */
    uint8_t SPI_DataSize;   /* Data frame size */
    uint8_t SPI_CPOL;       /* Clock polarity */
    uint8_t SPI_CPHA;       /* Clock phase */
    uint8_t SPI_SSM;        /* Software slave management */
} SPI_Config_t;

typedef struct{
	SPI_RegDef_t *pSPIx;
	SPI_Config_t SPIConfig;

	uint8_t *pTxBuffer;
	uint8_t *pRxBuffer;
	uint32_t TxLen;
	uint32_t RxLen;
	uint8_t TxState;
	uint8_t RxState;
}SPI_Handle_t;

/* ------------------------------------------------------------
 * SPI software states
 * ------------------------------------------------------------ */

#define SPI_READY        0U
#define SPI_BUSY_IN_RX   1U
#define SPI_BUSY_IN_TX   2U

/* ------------------------------------------------------------
 * SPI application events
 * ------------------------------------------------------------ */

#define SPI_EVENT_TX_CMPLT    1U
#define SPI_EVENT_RX_CMPLT    2U
#define SPI_EVENT_OVR_ERR     3U

/* ------------------------------------------------------------
 * SPI device mode
 * ------------------------------------------------------------ */

#define SPI_DEVICE_MODE_SLAVE	0U
#define SPI_DEVICE_MODE_MASTER  1U

/* ------------------------------------------------------------
 * SPI bus configuration
 * ------------------------------------------------------------ */

#define SPI_BUS_CONFIG_FD				1U  /* Full duplex */
#define SPI_BUS_CONFIG_HD				2U  /* Half duplex */
#define SPI_BUS_CONFIG_SIMPLEX_RXONLY	3U  /* Simplex receive-only */

/* ------------------------------------------------------------
 * SPI serial clock speed
 * ------------------------------------------------------------ */

#define SPI_SCLK_SPEED_DIV2      0U
#define SPI_SCLK_SPEED_DIV4      1U
#define SPI_SCLK_SPEED_DIV8      2U
#define SPI_SCLK_SPEED_DIV16     3U
#define SPI_SCLK_SPEED_DIV32     4U
#define SPI_SCLK_SPEED_DIV64     5U
#define SPI_SCLK_SPEED_DIV128    6U
#define SPI_SCLK_SPEED_DIV256    7U

/* ------------------------------------------------------------
 * SPI clock polarity and phase
 * ------------------------------------------------------------ */

#define SPI_CPOL_LOW	0U
#define SPI_CPOL_HIGH   1U

#define SPI_CPHA_LOW	0U
#define SPI_CPHA_HIGH   1U

/* ------------------------------------------------------------
 * SPI slave management
 * ------------------------------------------------------------ */

#define SPI_SSM_DI      0U
#define SPI_SSM_EN      1U

/* ------------------------------------------------------------
 * SPI data frame size
 * ------------------------------------------------------------ */

#define SPI_DS_8BITS     7U
#define SPI_DS_16BITS    15U

/* ------------------------------------------------------------
 * SPI driver APIs
 * ------------------------------------------------------------ */

void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);

void SPI_Init(SPI_Handle_t *pSPIHandle);
void SPI_DeInit(SPI_RegDef_t *pSPIx);

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len);
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len);

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len);

uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName);

void SPI_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi);
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
void SPI_IRQHandling(SPI_Handle_t *pHandle);

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);
void SPI_CloseTransmission(SPI_Handle_t *pHandle);
void SPI_CloseReception(SPI_Handle_t *pHandle);

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);

void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv);

#endif
