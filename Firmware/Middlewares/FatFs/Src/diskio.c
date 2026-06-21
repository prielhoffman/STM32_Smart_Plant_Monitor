/* USER CODE BEGIN Header */
/*
 * diskio.c
 *
 * FatFs low-level disk I/O layer for the MicroSD card.
 * This file connects FatFs to the custom SD card driver.
 */
/* USER CODE END Header */

#include "diskio.h"
#include "sd_card.h"

/*
 * FatFs uses drive numbers.
 * We only have one physical drive: the MicroSD card.
 */
#define DEV_SD      0U

/*
 * Sector size for SD cards.
 */
#define SD_SECTOR_SIZE_BYTES    512U

/*
 * We keep a simple local status.
 * STA_NOINIT means the disk is not initialized.
 */
static DSTATUS g_disk_status = STA_NOINIT;


DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != DEV_SD)
    {
        return STA_NOINIT;
    }

    /*
     * Important:
     * SPI GPIO, CS GPIO and SPI peripheral are initialized from main.c
     * before FatFs is used.
     *
     * SD_Card_InitCard() performs the SD protocol initialization:
     * CMD0, CMD8, ACMD41, CMD58.
     */
    if (SD_Card_InitCard())
    {
        g_disk_status = 0U;
    }
    else
    {
        g_disk_status = STA_NOINIT;
    }

    return g_disk_status;
}


DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != DEV_SD)
    {
        return STA_NOINIT;
    }

    return g_disk_status;
}


DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    UINT i = 0U;

    if (pdrv != DEV_SD)
    {
        return RES_PARERR;
    }

    if (buff == 0)
    {
        return RES_PARERR;
    }

    if (count == 0U)
    {
        return RES_PARERR;
    }

    if (g_disk_status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    /*
     * FatFs may ask for more than one sector.
     * Our driver currently reads one 512-byte block at a time,
     * so we loop over the requested sector count.
     */
    for (i = 0U; i < count; i++)
    {
        if (!SD_Card_ReadBlock((uint32_t)(sector + i),
                               &buff[i * SD_SECTOR_SIZE_BYTES]))
        {
            return RES_ERROR;
        }
    }

    return RES_OK;
}


#if _FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    UINT i = 0U;

    if (pdrv != DEV_SD)
    {
        return RES_PARERR;
    }

    if (buff == 0)
    {
        return RES_PARERR;
    }

    if (count == 0U)
    {
        return RES_PARERR;
    }

    if (g_disk_status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    /*
     * FatFs may write more than one sector.
     * Our driver writes one 512-byte block at a time.
     */
    for (i = 0U; i < count; i++)
    {
        if (!SD_Card_WriteBlock((uint32_t)(sector + i),
                                &buff[i * SD_SECTOR_SIZE_BYTES]))
        {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

#endif


DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT result = RES_OK;

    if (pdrv != DEV_SD)
    {
        return RES_PARERR;
    }

    if (g_disk_status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    switch (cmd)
    {
        case CTRL_SYNC:
            /*
             * Our write function already waits until the card is not busy.
             */
            result = RES_OK;
            break;

        case GET_SECTOR_SIZE:
            if (buff == 0)
            {
                result = RES_PARERR;
            }
            else
            {
                *(WORD *)buff = SD_SECTOR_SIZE_BYTES;
                result = RES_OK;
            }
            break;

        case GET_BLOCK_SIZE:
            if (buff == 0)
            {
                result = RES_PARERR;
            }
            else
            {
                /*
                 * Erase block size in units of sectors.
                 * A simple value of 1 is enough for basic FatFs operation.
                 */
                *(DWORD *)buff = 1U;
                result = RES_OK;
            }
            break;

        case GET_SECTOR_COUNT:
            if (buff == 0)
            {
                result = RES_PARERR;
            }
            else
            {
                /*
                 * Temporary approximate sector count.
                 * 32GB / 512 bytes = about 62,500,000 sectors.
                 *
                 * This is enough for mounting and creating a small CSV file.
                 * Later we can implement real CSD parsing if we want.
                 */
            	*(DWORD *)buff = 62500000U;
                result = RES_OK;
            }
            break;

        default:
            result = RES_PARERR;
            break;
    }

    return result;
}
