#ifndef INC_STM32G071XX_I2C_DRIVER_H_
#define INC_STM32G071XX_I2C_DRIVER_H_

#include "stm32g071xx.h"

/* ------------------------------------------------------------
 * Configuration structures
 * ------------------------------------------------------------ */

typedef struct{
	uint32_t I2C_SCLSpeed;
	uint8_t I2C_DeviceAddress;
	uint8_t I2C_ACKControl;
	uint8_t I2C_FMDutyCycle;
}I2C_Config_t;

typedef struct{
	I2C_RegDef_t *pI2Cx;
	I2C_Config_t I2CConfig;

	uint8_t *pTxBuffer;
	uint8_t *pRxBuffer;
	uint32_t TxLen;
	uint32_t RxLen;
	uint8_t TxRxState;
	uint8_t DevAddr;
	uint32_t RxSize;
	uint8_t Sr;
}I2C_Handle_t;

/* ------------------------------------------------------------
 * I2C SCL speed options
 * ------------------------------------------------------------ */

#define I2C_SCL_SPEED_SM      100000U
#define I2C_SCL_SPEED_FM2K    200000U
#define I2C_SCL_SPEED_FM4K    400000U

/* ------------------------------------------------------------
 * I2C ACK control
 * ------------------------------------------------------------ */

#define I2C_ACK_ENABLE        1U
#define I2C_ACK_DISABLE       0U

/* ------------------------------------------------------------
 * I2C fast-mode duty cycle
 * ------------------------------------------------------------ */

#define I2C_FM_DUTY_2         0U
#define I2C_FM_DUTY_16_9      1U

/* ------------------------------------------------------------
 * I2C software states
 * ------------------------------------------------------------ */

#define I2C_READY       0U
#define I2C_BUSY_IN_RX  1U
#define I2C_BUSY_IN_TX  2U

/* ------------------------------------------------------------
 * Repeated-start control
 * ------------------------------------------------------------ */

#define I2C_DISABLE_SR  0U
#define I2C_ENABLE_SR   1U

/* ------------------------------------------------------------
 * I2C application events
 * ------------------------------------------------------------ */

#define I2C_EV_TX_CMPLT   0U
#define I2C_EV_RX_CMPLT   1U
#define I2C_EV_STOP       2U

#define I2C_ERROR_AF      3U
#define I2C_ERROR_BERR    4U
#define I2C_ERROR_ARLO    5U
#define I2C_ERROR_OVR     6U

#define I2C_EV_DATA_REQ	  7U
#define I2C_EV_DATA_RCV   8U

/* ------------------------------------------------------------
 * I2C driver APIs
 * ------------------------------------------------------------ */

void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);

void I2C_Init(I2C_Handle_t *pI2CHandle);
void I2C_DeInit(I2C_RegDef_t *pI2Cx);

void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName);

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr);
void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr);

void I2C_SlaveSendData(I2C_RegDef_t *pI2Cx, uint8_t data);
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2Cx);

void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);

uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr);
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr);

void I2C_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi);
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
void I2C_IRQHandling(I2C_Handle_t *pI2CHandle);
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv);

#endif
