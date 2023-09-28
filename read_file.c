#include "HAL.h"
#include "read_file.h"
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
void readBootSector(Boot *boot, uint8_t *buff)
{
    HALReadSector(0, buff);
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
static Directory convertDateTime(uint16_t date, uint16_t time)
{
    Directory result;
    result.day = date & 0x1F;
    result.month = (date >> 5) & 0x0F;
    result.year = ((date >> 9) & 0x7F) + 1980;
    result.second = (time & 0x1F) * 2;
    result.minute = (time >> 5) & 0x3F;
    result.hour = (time >> 11) & 0x1F;
    return result;
}
void printDirectory(Directory *directory, uint8_t index)
{
    Directory result = convertDateTime(directory->date, directory->time);
    printf("%-2d: %-10s  %-4s %10d ", index, directory->nameOfFile, directory->extension, directory->fileSize);
    printf("\t%02d/%02d/%04d\t%02d:%02d:%02d %x\n", result.day, result.month, result.year, result.hour, result.minute, result.second, directory->clusterNumber);
}

void readFileName(uint8_t *buff, uint8_t *fileName)
{
    for (uint8_t i = 0; i < 9; i++)
    {
        if (buff[i] == SPACE_CHARACTER)
        {
            buff[i] = '\0';
        }
        fileName[i] = buff[i];
    }
    fileName[8] = '\0';
}

void readExtension(uint8_t *buff, uint8_t *extension)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        extension[i] = buff[8 + i];
    }
    extension[3] = '\0';
}

void convertEntryToDirectory(Directory *directory, uint8_t *buff, uint32_t index)
{
    readFileName(&buff[index], directory->nameOfFile);
    readExtension(&buff[index], directory->extension);
    directory->time = CONVERT_2_BYTES(&buff[index + ENTRY_TIME_OFFSET]);
    directory->date = CONVERT_2_BYTES(&buff[index + ENTRY_DATE_OFFSET]);
    directory->clusterNumber = CONVERT_2_BYTES(&buff[index + ENTRY_CLUSTER_OFFSET]);
    directory->fileSize = CONVERT_4_BYTES(&buff[index + ENTRY_FILESIZE_OFFSET]);
}

void readRootDirectory(Directory *directory, uint8_t *buff, uint32_t *cluster, uint32_t *attribute)
{   
    uint32_t readMultiByte = HALReadMultilSector(RootDirStartSector,RootDirSector, buff);
    // uint32_t readMultiByte = HALReadMultilSector(38,1, buff);
    uint32_t maxEntries = readMultiByte / SIZE_ENTRY; // 14
    directory->clusterIndex = 0;
    uint8_t index = 1;
    for (uint32_t j = 0; j < maxEntries; j++)
    {
        uint32_t entryIndex = j * SIZE_ENTRY;
        if (buff[entryIndex] == 0)
        {
            return;
        }
        else if (buff[entryIndex + 11] != 0x0F)
        {
            convertEntryToDirectory(directory, buff, entryIndex);
            directory->clusterIndex++;
            cluster[directory->clusterIndex] = directory->clusterNumber;
            attribute[directory->clusterIndex] = buff[entryIndex + 11];
            printDirectory(directory, index);
            index++;
        }
    }
}
// void readSubDirectory(Directory *directory, uint8_t *buff, uint32_t cluster, uint32_t *attribute)
// {
//     uint32_t offsetData = 33 + cluster - 2; 
//     uint32_t readMultiByte = HALReadSector(offsetData, buff);
//     // uint32_t readMultiByte = HALReadMultilSector(38,1, buff);
//     uint32_t maxEntries = readMultiByte / SIZE_ENTRY; // 14
//     directory->clusterIndex = 0;
//     uint8_t index = 1;
//     for (uint32_t j = 0; j < maxEntries; j++)
//     {
//         uint32_t entryIndex = j * SIZE_ENTRY;
//         if (buff[entryIndex] == 0)
//         {
//             return;
//         }
//         else if (buff[entryIndex + 11] != 0x0F)
//         {
//             convertEntryToDirectory(directory, buff, entryIndex);
//             directory->clusterIndex++;
//             cluster[directory->clusterIndex] = directory->clusterNumber;
//             attribute[directory->clusterIndex] = buff[entryIndex + 11];
//             printDirectory(directory, index);
//             index++;
//         }
//     }
// }

uint32_t readFAT(uint8_t *buff, uint32_t cluster)
{
    uint32_t byte = HALReadMultilSector(1, 9, buff);
    uint32_t fatOffset = (33 + cluster - 2) * 512;
    uint32_t result = 0;
    if (cluster % 2 != 0)
    {
        result = (buff[fatOffset] >> 4) | ((uint16_t)buff[fatOffset + 1] << 4);
    }
    else
    {
        result = (buff[fatOffset]) | ((uint16_t)buff[fatOffset + 1] & 0x0F) << 8;
    }
    return result;
}


uint32_t readData(Data *data, uint8_t *buff, uint32_t cluster)
{

    uint32_t dataStartSector = RootDirStartSector + RootDirSector; // 33
    uint32_t dataSector = TOTAL_SECTOR - dataStartSector;          // 2847
    uint32_t offsetData = DATA_START_SECTOR + cluster - 2;         // 33
    uint32_t readSector = HALReadSector(offsetData, buff);
    return readSector;
}