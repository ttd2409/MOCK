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
    }
    else
    {

        fseek(file, sectorAddress, SEEK_SET); // Di chuyển con trỏ tới địa chỉ của sector cần đọc
        for (uint32_t i = 0; i < num; i++)
        {
            uint32_t sectorRead = fread(buff + (i * SECTOR_SIZE), sizeof(uint8_t), SECTOR_SIZE, file);
            byteRead += sectorRead;
        }
    }
    fclose(file);
    return byteRead;
}

uint32_t HALReadMultiByte(uint32_t index, uint32_t num, uint8_t *buff)
{
    uint32_t byteRead = 0;
    uint32_t sectorAddress = index * SECTOR_SIZE;
    FILE *file = fopen(FILE_NAME, FILE_MODE);
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return -1;
    }
    else
    {
        fseek(file, sectorAddress, sizeof(uint8_t));
        for (uint32_t i = 0; i < num; i++)
        {
            uint32_t sectorRead = fread(buff, sizeof(uint8_t), num, file);
            byteRead += sectorRead;
        }
    }
    fclose(file);
    return byteRead;
}