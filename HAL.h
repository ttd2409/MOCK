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

#define FILE_NAME "floppy.img"
#define FILE_MODE "rb"
#define SECTOR_SIZE 512U
#define FAT_SIZE 512 * 9
#define ROOT_SIZE 512 * 14
#define DATA_SIZE 512 * 2847
#define MAX_BYTE 512 * 2879
#define SIZE_ENTRY    32U
#define CONVERT_2_BYTES(x) (((*((x) + 1)) << 8)|(*(x)))
#define CONVERT_4_BYTES(x) (((*((x) + 3))<<24)|((*((x) + 2)) << 16)|((*((x) + 1)) << 8)|(*(x)))

typedef enum
{
    ERROR_OPEN,

}ERRO_FILE;

uint32_t HALReadSector(uint32_t index, uint8_t *buff);
uint32_t HALReadMultilSector(uint32_t index, uint32_t num, uint8_t *buff);
uint32_t HALReadMultiByte(uint32_t index, uint32_t num, uint8_t *buff);

#endif 