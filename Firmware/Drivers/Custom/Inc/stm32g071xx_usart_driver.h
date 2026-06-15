#ifndef INC_STM32G071XX_USART_DRIVER_H_
#define INC_STM32G071XX_USART_DRIVER_H_

#include "stm32g071xx.h"

/* ------------------------------------------------------------
 * Configuration structures
 * ------------------------------------------------------------ */

typedef struct
{
    uint8_t  USART_Mode;
    uint32_t USART_Baud;
    uint8_t  USART_NoOfStopBits;
    uint8_t  USART_WordLength;
    uint8_t  USART_ParityControl;
    uint8_t  USART_HWFlowControl;
} USART_Config_t;

typedef struct
{
    USART_RegDef_t *pUSARTx;
    USART_Config_t USARTConfig;

    uint8_t *pTxBuffer;
    uint8_t *pRxBuffer;
    uint32_t TxLen;
    uint32_t RxLen;
    uint8_t TxState;
    uint8_t RxState;
} USART_Handle_t;

/* ------------------------------------------------------------
 * USART mode configuration
 * ------------------------------------------------------------ */

/*
 * Encoded so the values can be mapped directly to RE/TE bits:
 * RX   = 01
 * TX   = 10
 * TXRX = 11
 */
#define USART_MODE_ONLY_RX       1U
#define USART_MODE_ONLY_TX       2U
#define USART_MODE_TXRX          3U

/* ------------------------------------------------------------
 * USART baud rate options
 * ------------------------------------------------------------ */

#define USART_STD_BAUD_1200       1200U
#define USART_STD_BAUD_2400       2400U
#define USART_STD_BAUD_9600       9600U
#define USART_STD_BAUD_19200      19200U
#define USART_STD_BAUD_38400      38400U
#define USART_STD_BAUD_57600      57600U
#define USART_STD_BAUD_115200     115200U
#define USART_STD_BAUD_230400     230400U
#define USART_STD_BAUD_460800     460800U
#define USART_STD_BAUD_921600     921600U
#define USART_STD_BAUD_2000000    2000000U
#define USART_STD_BAUD_3000000    3000000U

/* ------------------------------------------------------------
 * USART stop bits
 * ------------------------------------------------------------ */

#define USART_STOPBITS_1          0U
#define USART_STOPBITS_0_5        1U
#define USART_STOPBITS_2          2U
#define USART_STOPBITS_1_5        3U

/* ------------------------------------------------------------
 * USART word length
 * ------------------------------------------------------------ */

#define USART_WORDLEN_8BITS       0U
#define USART_WORDLEN_9BITS       1U
#define USART_WORDLEN_7BITS       2U

/* ------------------------------------------------------------
 * USART parity control
 * ------------------------------------------------------------ */

#define USART_PARITY_DISABLE      0U  /* 00: PCE=0, PS=0 */
#define USART_PARITY_EN_EVEN      2U  /* 10: PCE=1, PS=0 */
#define USART_PARITY_EN_ODD       3U  /* 11: PCE=1, PS=1 */

/* ------------------------------------------------------------
 * USART hardware flow control
 * ------------------------------------------------------------ */

#define USART_HW_FLOW_CTRL_NONE       0U
#define USART_HW_FLOW_CTRL_RTS		  1U
#define USART_HW_FLOW_CTRL_CTS        2U
#define USART_HW_FLOW_CTRL_CTS_RTS    3U

/* ------------------------------------------------------------
 * USART application events
 * ------------------------------------------------------------ */

#define USART_EVENT_TX_CMPLT    0U
#define USART_EVENT_RX_CMPLT    1U
#define USART_EVENT_ORE         2U
#define USART_EVENT_IDLE        3U
#define USART_EVENT_NE          4U
#define USART_EVENT_FE          5U

/* ------------------------------------------------------------
 * USART driver APIs
 * ------------------------------------------------------------ */

void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi);

void USART_Init(USART_Handle_t *pHandle);
void USART_DeInit(USART_RegDef_t *pUSARTx);

void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi);
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint32_t FlagName);
void USART_ClearFlag(USART_RegDef_t *pUSARTx, uint16_t StatusFlagName);

void USART_SendData(USART_Handle_t *pHandle, uint8_t *pTxBuffer, uint32_t Len);
void USART_ReceiveData(USART_Handle_t *pHandle, uint8_t *pRxBuffer, uint32_t Len);

uint8_t USART_SendDataIT(USART_Handle_t *pHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t USART_ReceiveDataIT(USART_Handle_t *pHandle, uint8_t *pRxBuffer, uint32_t Len);

void USART_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi);
void USART_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
void USART_IRQHandling(USART_Handle_t *pHandle);

void USART_ClearOREFlag(USART_RegDef_t *pUSARTx);
void USART_CloseTransmission(USART_Handle_t *pHandle);
void USART_CloseReception(USART_Handle_t *pHandle);

void USART_ApplicationEventCallback(USART_Handle_t *pHandle, uint8_t AppEv);

#endif
