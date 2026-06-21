#include "sd_card.h"

#include <stddef.h>

#define SD_DATA_TOKEN_START_BLOCK   0xFEU
#define SD_BLOCK_SIZE               512U

static SPI_Handle_t *g_sd_spi_handle = NULL;
static GPIO_RegDef_t *g_sd_cs_gpio_port = NULL;
static uint8_t g_sd_cs_pin = 0U;
static uint8_t g_sd_card_type = SD_CARD_TYPE_UNKNOWN;

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

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc){
    uint8_t response = 0xFFU;

    SD_CS_LOW();

    /*
     * SD command packet is always 6 bytes:
     * byte 0: command index with start bits
     * bytes 1-4: 32-bit argument, MSB first
     * byte 5: CRC
     */
    SD_SPI_TransferByte((uint8_t)(0x40U | cmd));
    SD_SPI_TransferByte((uint8_t)(arg >> 24U));
    SD_SPI_TransferByte((uint8_t)(arg >> 16U));
    SD_SPI_TransferByte((uint8_t)(arg >> 8U));
    SD_SPI_TransferByte((uint8_t)arg);
    SD_SPI_TransferByte(crc);

    /*
     * Read response
     * The card may return 0xFF for a few bytes before the real response
     */
    for (uint8_t i = 0U; i < 8U; i++){
        response = SD_SPI_TransferByte(0xFFU);

        if (response != 0xFFU){
            break;
        }
    }

    return response;
}

static uint8_t SD_SendCMD8(uint8_t *r7_response){
    uint8_t response = 0xFFU;

    if (r7_response == NULL){
        return 0xFFU;
    }

    /*
     * CMD8:
     * arg = 0x000001AA
     * 0x1 = voltage supplied range
     * 0xAA = check pattern
     * crc = 0x87
     */
    response = SD_SendCommand(8U, 0x000001AAU, 0x87U);

    /* R7 response includes 4 more bytes after the first response byte */
    for (uint8_t i = 0U; i < 4U; i++){
        r7_response[i] = SD_SPI_TransferByte(0xFFU);
    }

    SD_CS_HIGH();
    SD_SPI_TransferByte(0xFFU);

    return response;
}

static uint8_t SD_SendCMD55(void){
    uint8_t response = 0xFFU;

    response = SD_SendCommand(55U, 0x00000000U, 0x01U);

    SD_CS_HIGH();
    SD_SPI_TransferByte(0xFFU);

    return response;
}

static uint8_t SD_SendACMD41(void){
    uint8_t response = 0xFFU;

    /*
     * ACMD41 with HCS bit set
     * HCS tells the card that the host supports SDHC/SDXC
     */
    response = SD_SendCommand(41U, 0x40000000U, 0x01U);

    SD_CS_HIGH();
    SD_SPI_TransferByte(0xFFU);

    return response;
}

static uint8_t SD_SendCMD58(uint8_t *ocr){
    uint8_t response = 0xFFU;

    if (ocr == NULL){
        return 0xFFU;
    }

    response = SD_SendCommand(58U, 0x00000000U, 0x01U);

    for (uint8_t i = 0U; i < 4U; i++){
        ocr[i] = SD_SPI_TransferByte(0xFFU);
    }

    SD_CS_HIGH();
    SD_SPI_TransferByte(0xFFU);

    return response;
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

    SD_SendInitialClocks();

    /*
     * CMD0:
     * arg = 0
     * crc = 0x95
     */
    response = SD_SendCommand(0U, 0x00000000U, 0x95U);

    SD_CS_HIGH();
    SD_SPI_TransferByte(0xFFU);

    return response;
}

uint8_t SD_Card_InitCard(void){
    uint8_t response = 0xFFU;
    uint8_t r7[4] = {0};
    uint8_t ocr[4] = {0};
    uint16_t retry = 0U;

    if ((g_sd_spi_handle == NULL) || (g_sd_cs_gpio_port == NULL)){
        return 0U;
    }

    /* Step 1: CMD0 - reset card and enter idle state */
    response = SD_Card_SendCMD0();
    if (response != SD_RESPONSE_IDLE_STATE){
        return 0U;
    }

    /* Step 2: CMD8 - check SD version and voltage range */
    response = SD_SendCMD8(r7);
    if (response != SD_RESPONSE_IDLE_STATE){
        return 0U;
    }
    /*
     * Expected R7 pattern:
     * r7[2] = 0x01
     * r7[3] = 0xAA
     */
    if ((r7[2] != 0x01U) || (r7[3] != 0xAAU)){
        return 0U;
    }

    /*
     * Step 3: ACMD41 - initialize card
     * The card may stay busy for a while, so retry
     */
    for (retry = 0U; retry < 1000U; retry++){
        response = SD_SendCMD55();
        if ((response != SD_RESPONSE_IDLE_STATE) && (response != SD_RESPONSE_READY)){
            return 0U;
        }
        response = SD_SendACMD41();
        if (response == SD_RESPONSE_READY){
            break;
        }
    }
    if (response != SD_RESPONSE_READY){
        return 0U;
    }

    /* Step 4: CMD58 - read OCR and detect SDHC */
    response = SD_SendCMD58(ocr);
    if (response != SD_RESPONSE_READY){
        return 0U;
    }
    /* OCR[0] bit 6 corresponds to OCR bit 30, CCS */
    if (ocr[0] & 0x40U){
        g_sd_card_type = SD_CARD_TYPE_SDHC;
    }
    else{
        g_sd_card_type = SD_CARD_TYPE_UNKNOWN;
    }

    return 1U;
}

uint8_t SD_Card_GetType(void){
    return g_sd_card_type;
}

uint8_t SD_Card_ReadBlock(uint32_t block_addr, uint8_t *rx_buffer){
    uint8_t response = 0xFFU;
    uint8_t token = 0xFFU;
    uint16_t timeout = 0U;

    if ((g_sd_spi_handle == NULL) || (g_sd_cs_gpio_port == NULL) || (rx_buffer == NULL)){
        return 0U;
    }

    /*
     * For SDHC cards, the address argument is a block number
     * Our card was detected as SDHC, so block_addr is used directly
     */
    if (g_sd_card_type != SD_CARD_TYPE_SDHC){
        return 0U;
    }

    /*
     * CMD17 = READ_SINGLE_BLOCK
     * For SDHC, argument is the block number
     */
    response = SD_SendCommand(17U, block_addr, 0x01U);
    if (response != SD_RESPONSE_READY){
        SD_CS_HIGH();
        SD_SPI_TransferByte(0xFFU);
        return 0U;
    }

    /*
     * Wait for data token 0xFE
     * The card may need some time before it starts sending the block
     */
    for (timeout = 0U; timeout < 50000U; timeout++){
        token = SD_SPI_TransferByte(0xFFU);
        if (token == SD_DATA_TOKEN_START_BLOCK){
            break;
        }
    }

    if (token != SD_DATA_TOKEN_START_BLOCK){
        SD_CS_HIGH();
        SD_SPI_TransferByte(0xFFU);
        return 0U;
    }

    /* Read 512 bytes of block data */
    for (uint16_t i = 0U; i < SD_BLOCK_SIZE; i++){
        rx_buffer[i] = SD_SPI_TransferByte(0xFFU);
    }

    /*
     * Read and ignore 2 CRC bytes
     * CRC is not used here
     */
    SD_SPI_TransferByte(0xFFU);
    SD_SPI_TransferByte(0xFFU);

    SD_CS_HIGH();
    SD_SPI_TransferByte(0xFFU);

    return 1U;
}
