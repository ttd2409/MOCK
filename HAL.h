#ifndef _HAL_H_
#define _HAL_H_
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define SECTOR_SIZE 512U
#define FAT_SIZE 512 * 9
#define ROOT_SIZE 512 * 14
#define DATA_SIZE 512 * 2847
#define MAX_BYTE 512 * 2879
#define SIZE_ENTRY    32U


typedef struct BOOT_SECTER
{
    uint16_t bytePerSector;
    uint16_t totalSector;
    uint16_t sectorPerFAT;
    uint16_t rootEntryCount;
    uint8_t sectorsPerCluster;
    uint8_t numberReservedSectors;
    uint8_t FATTableNumber;
} Boot;
typedef struct FAT
{
    uint8_t fatStartSector;
    uint8_t fatSector;

} FAT;

uint32_t convertLittleToBig32(uint32_t value);
uint16_t convertLittleToBig16(uint16_t value);
uint32_t HALReadSector(uint32_t index, uint8_t *buff, FILE *file);
uint32_t HALReadMultilSector(uint32_t index, uint32_t num, uint8_t *buff, FILE *file);
void readBootSector(Boot * boot, FILE* fp, uint8_t* buff);

#endif /*_HAL_H_*/

