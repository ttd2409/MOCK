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

uint32_t HALReadSector(uint32_t index, uint8_t *buff)
{
    uint32_t byteRead;
    FILE *file = fopen(FILE_NAME, FILE_MODE);
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return -1;
    }
    fseek(file, index * SECTOR_SIZE, SEEK_SET);
    byteRead = fread(buff, sizeof(uint8_t), SECTOR_SIZE, file);
    fclose(file);
    return byteRead;
}

uint32_t HALReadMultilSector(uint32_t index, uint32_t num, uint8_t *buff)
{
    uint32_t sectorAddress = index * SECTOR_SIZE;
    uint32_t byteRead = 0;
    FILE *file = fopen(FILE_NAME, FILE_MODE);
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return -1;
    }
    fseek(file, sectorAddress, SEEK_SET); // Di chuyển con trỏ tới địa chỉ của sector cần đọc
    for (uint32_t i = 0; i < num; i++)
    {
        uint32_t sectorRead = fread(buff + (i * SECTOR_SIZE), sizeof(uint8_t), SECTOR_SIZE, file);
        byteRead += sectorRead;
    }
    fclose(file);
    return byteRead;
}

void readBootSector(Boot *boot, uint8_t *buff)
{
    int byteCount = HALReadSector(0, buff);
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
