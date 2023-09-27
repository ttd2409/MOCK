#include "HAL.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Code
 ******************************************************************************/
/*******************************************************************************
 * API
 ******************************************************************************/

uint32_t HALReadSector(uint32_t index, uint8_t *buff, FILE *fp)
{
    uint32_t byteRead;
    fseek(fp, index * SECTOR_SIZE, SEEK_SET);
    byteRead = fread(buff, sizeof(uint8_t), SECTOR_SIZE, fp);
    return byteRead;
}

uint32_t HALReadMultilSector(uint32_t index, uint32_t num, uint8_t *buff, FILE *fp)
{
    uint32_t sectorAddress = index * SECTOR_SIZE;
    uint32_t byteRead = 0;
    fseek(fp, sectorAddress, SEEK_SET); // Di chuyển con trỏ tới địa chỉ của sector cần đọc
    for (uint32_t i = 0; i < num; i++)
    {
        uint32_t sectorRead = fread(buff + (i * SECTOR_SIZE), sizeof(uint8_t), SECTOR_SIZE, fp);
        byteRead += sectorRead;
    }
    return byteRead;
}

void readBootSector(Boot *boot, FILE *fp, uint8_t *buff)
{
    int byteCount = HALReadSector(0, buff, fp);
    if (buff[510] != 0x55 || buff[511] != 0xAA)
    {
        printf("Invalid Boot Sector\n");
    }
    else
    {
        boot->bytePerSector = CONVERT_2_BYTES(&buff[11]);
        boot->sectorsPerCluster = buff[13];
        boot->numberReservedSectors = buff[14];
        boot->FATTableNumber = buff[16];
        boot->rootEntryCount = CONVERT_2_BYTES(&buff[17]);
        boot->totalSector = CONVERT_2_BYTES(&buff[19]);
        boot->sectorPerFAT = buff[22];
    }
}

