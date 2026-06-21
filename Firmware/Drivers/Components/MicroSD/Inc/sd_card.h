#ifndef SD_CARD_H_
#define SD_CARD_H_

#include <stdint.h>

#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_spi_driver.h"

/*
 * SD card basic response values
 * CMD0 should return 0x01 when the card enters idle state successfully
 */
#define SD_CMD0_RESPONSE_IDLE_STATE     0x01U

/*
 * Initialize the MicroSD component driver
 * This function does not initialize SPI GPIO pins or the SPI peripheral
 * Those are board-specific and should be done in main.c
 *
 * It only stores:
 * which SPI peripheral is used
 * which GPIO pin is used as CS
 */
void SD_Card_Init(SPI_Handle_t *pSPIHandle, GPIO_RegDef_t *pCSGPIOPort, uint8_t cs_pin);

/*
 * Send CMD0 to the SD card
 * CMD0 resets the card and asks it to enter SPI idle state
 * Expected successful response:
 * 0x01
 */
uint8_t SD_Card_SendCMD0(void);

#endif
