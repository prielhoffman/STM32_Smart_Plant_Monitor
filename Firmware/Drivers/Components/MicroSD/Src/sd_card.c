#include "sd_card.h"

#include <stddef.h>

static SPI_Handle_t *g_sd_spi_handle = NULL;
static GPIO_RegDef_t *g_sd_cs_gpio_port = NULL;
static uint8_t g_sd_cs_pin = 0U;

static void SD_CS_LOW(void){
    GPIO_WriteToOutputPin(g_sd_cs_gpio_port, g_sd_cs_pin, GPIO_PIN_RESET);
}

static void SD_CS_HIGH(void){
    GPIO_WriteToOutputPin(g_sd_cs_gpio_port, g_sd_cs_pin, GPIO_PIN_SET);
}

/*
 * Transfer one byte over SPI
 * For SD cards, we often send 0xFF just to generate clock pulses
 * and read the response from the card
 */
static uint8_t SD_SPI_TransferByte(uint8_t tx_byte){
    SPI_RegDef_t *pSPIx;
    uint8_t rx_byte = 0xFFU;

    if (g_sd_spi_handle == NULL){
        return 0xFFU;
    }

    pSPIx = g_sd_spi_handle->pSPIx;

    /* Wait until transmit buffer is empty */
    while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);

    /* Write one byte to SPI data register */
    *((__vo uint8_t *)&pSPIx->DR) = tx_byte;

    /* Wait until one byte is received */
    while (SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

    /* Read received byte */
    rx_byte = *((__vo uint8_t *)&pSPIx->DR);

    return rx_byte;
}

/*
 * Send at least 74 clock cycles with CS high
 * SD card initialization requires this before the first command
 * We send 10 bytes of 0xFF:
 * 10 bytes * 8 bits = 80 clock cycles
 */
static void SD_SendInitialClocks(void){
    SD_CS_HIGH();

    for (uint8_t i = 0U; i < 10U; i++){
        SD_SPI_TransferByte(0xFFU);
    }
}

void SD_Card_Init(SPI_Handle_t *pSPIHandle, GPIO_RegDef_t *pCSGPIOPort, uint8_t cs_pin){
    g_sd_spi_handle = pSPIHandle;
    g_sd_cs_gpio_port = pCSGPIOPort;
    g_sd_cs_pin = cs_pin;

    /*
     * CS is active-low
     * Keep the card deselected by default
     */
    SD_CS_HIGH();
}

uint8_t SD_Card_SendCMD0(void){
    uint8_t response = 0xFFU;

    if ((g_sd_spi_handle == NULL) || (g_sd_cs_gpio_port == NULL)){
        return 0xFFU;
    }

    /* Before CMD0, the card needs initial clocks with CS high */
    SD_SendInitialClocks();

    /* Select the SD card */
    SD_CS_LOW();

    /*
     * CMD0 packet:
     * Byte 0: 0x40 = CMD0
     * Bytes 1-4: argument = 0x00000000
     * Byte 5: 0x95 = valid CRC for CMD0
     * CRC is usually disabled in SPI mode after initialization,
     * but CMD0 requires a valid CRC.
     */
    SD_SPI_TransferByte(0x40U);
    SD_SPI_TransferByte(0x00U);
    SD_SPI_TransferByte(0x00U);
    SD_SPI_TransferByte(0x00U);
    SD_SPI_TransferByte(0x00U);
    SD_SPI_TransferByte(0x95U);

    /*
     * The card may not respond immediately
     * Read up to 8 bytes until the response is not 0xFF
     */
    for (uint8_t i = 0U; i < 8U; i++){
        response = SD_SPI_TransferByte(0xFFU);

        if (response != 0xFFU){
            break;
        }
    }

    /* Deselect the card */
    SD_CS_HIGH();

    /* Send one extra 0xFF after CS high to release the bus */
    SD_SPI_TransferByte(0xFFU);

    return response;
}
