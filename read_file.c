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
static Date convertDate(uint16_t data)
{
    Date result;
    result.day = data & 0x1F;
    result.month = (data >> 5) & 0x0F;
    result.year = ((data >> 9) & 0x7F) + 1980;
    return result;
}
static Time convertTime(uint16_t data)
{
    Time result;
    result.second = (data & 0x1F) * 2;
    result.minute = (data >> 5) & 0x3F;
    result.hour = (data >> 11) & 0x1F;
    return result;
}
void printDirectory(Directory *directory, bool isFolder, uint8_t index)
{
    Date date = convertDate(directory->date);
    Time time = convertTime(directory->time);
    if (isFolder)
    {
        printf("%d: %s%s\t\t\t", index, directory->nameOfFile, directory->extension);
        printf("%02d/%02d/%04d\t%02d:%02d:%02d\t%x\n", date.day, date.month, date.year, time.hour, time.minute, time.second, directory->clusterNumber);
    }
    else
    {
        printf("%d: %s.%s\t\t%d\t", index, directory->nameOfFile, directory->extension, directory->fileSize);
        printf("%02d/%02d/%04d\t%02d:%02d:%02d\t%x\n", date.day, date.month, date.year, time.hour, time.minute, time.second, directory->clusterNumber);
    }
}

void readFileName(uint8_t *buff, uint8_t *fileName)
{
    for (uint8_t i = 0; i < 8; i++)
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

void readDirectory(Directory *directory, FILE *file, uint8_t *buff, uint32_t *cluster,uint32_t * attribute)
{
    uint32_t readMultiByte = HALReadMultilSector(RootDirStartSector, RootDirSector, buff, file);
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
        else if (buff[entryIndex+11] != 0x0F)
        {
            convertEntryToDirectory(directory, buff, entryIndex);
            directory->clusterIndex++;
            cluster[directory->clusterIndex] = directory->clusterNumber;
            attribute[directory->clusterIndex] = buff[entryIndex + 11];

            if (buff[entryIndex + 11] == 0x10) // folder
            {
                printDirectory(directory, true, index);
            }
            else if (buff[entryIndex + 11] == 0) // file
            {
                printDirectory(directory, false,index);
            }
            index++;
        }
    }
}
void readSubDirectory(Directory * directory, uint32_t *cluster)
{
    // uint32_t offsetDATA = (DATA_START_SECTOR + cluster - 2 )* 512;
}
void readFAT(uint8_t *buff, FILE *file)
{
    
}
void readData(Data *data, uint8_t *buff, FILE *file, uint32_t cluster)
{
    uint32_t dataStartSector = RootDirStartSector + RootDirSector; // 33
    uint32_t dataSector = TOTAL_SECTOR - dataStartSector;    // 2847
    uint32_t offsetData = DATA_START_SECTOR + cluster - 2; //33 
    uint32_t readSector = HALReadSector(offsetData, buff, file);
    for (int i = 0; i < readSector; ++i)
    {
        printf("%c", buff[i]);
    }
}