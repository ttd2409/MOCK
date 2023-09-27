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
static DateTime convertDateTime(uint16_t date, uint16_t time)
{
    DateTime result;
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
    DateTime result = convertDateTime(directory->date, directory->time);
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

void readDirectory(Directory *directory, uint8_t *buff, uint32_t *cluster, uint32_t *attribute)
{
    uint32_t readMultiByte = HALReadMultilSector(RootDirStartSector, RootDirSector, buff);
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
            printDirectory(directory,index);
            index++;
        }
    }
}
void readSubDirectory(Directory *directory, uint32_t *cluster)
{
    // uint32_t offsetDATA = (DATA_START_SECTOR + cluster - 2 )* 512;
}
void readFAT(uint8_t *buff)
{
}
uint32_t readData(Data *data, uint8_t *buff, uint32_t cluster)
{
    uint32_t dataStartSector = RootDirStartSector + RootDirSector; // 33
    uint32_t dataSector = TOTAL_SECTOR - dataStartSector;          // 2847
    uint32_t offsetData = DATA_START_SECTOR + cluster - 2;         // 33
    uint32_t readSector = HALReadSector(offsetData, buff);
    // for (int i = 0; i < readSector; ++i)
    // {
    //     printf("%c", buff[i]);
    // }
    return readSector;
}