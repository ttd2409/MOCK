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
void printDirectory(Directory *directory, bool isFolder)
{
    static uint8_t index = 1;
    Date date = convertDate(directory->date);
    Time time = convertTime(directory->time);
    if (isFolder)
    {
        printf("%d: %s%s\t\t\t", index++, directory->nameOfFile, directory->extension);
        printf("%02d/%02d/%04d\t%02d:%02d:%02d\n", date.day, date.month, date.year, time.hour, time.minute, time.second);
    }
    else
    {
        printf("%d: %s.%s\t\t%d\t", index++, directory->nameOfFile, directory->extension, directory->fileSize);
        printf("%02d/%02d/%04d\t%02d:%02d:%02d\n", date.day, date.month, date.year, time.hour, time.minute, time.second);
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
    directory->time = convertLittleToBig16((buff[index + ENTRY_TIME_OFFSET] << 8) | buff[index + ENTRY_TIME_OFFSET + 1]);
    directory->date = convertLittleToBig16((buff[index + ENTRY_DATE_OFFSET] << 8) | buff[index + ENTRY_DATE_OFFSET + 1]);
    directory->clusterNumber = convertLittleToBig16((buff[index + ENTRY_CLUSTER_OFFSET] << 8) | buff[index + ENTRY_CLUSTER_OFFSET + 1]);
    directory->fileSize = convertLittleToBig32((buff[index + ENTRY_FILESIZE_OFFSET] << 24) | (buff[index + ENTRY_FILESIZE_OFFSET + 1] << 16) | 
                    (buff[index + ENTRY_FILESIZE_OFFSET + 2] << 8) | buff[index + ENTRY_FILESIZE_OFFSET + 3]);
}


void readDirectory(Directory *directory, FILE *file, uint8_t *buff)
{
    uint32_t readMultiByte = HALReadMultilSector(RootDirStartSector, RootDirSector, buff, file);
    uint32_t maxEntries = readMultiByte / SIZE_ENTRY;
    for (uint32_t j = 0; j < maxEntries; j++)
    {
        uint32_t index = j * SIZE_ENTRY;
        if (buff[index] != 0)
        {
            convertEntryToDirectory(directory, buff, index);
            if (buff[index + 11] == 0x10) // folder
            {
                printDirectory(directory, true);
            }
            else if (buff[index + 11] == 0) // file
            {
                printDirectory(directory, false);
            }
        }
    }
}
void readFAT(uint8_t *buff, FILE *file)
{
}
void readData(uint8_t *buff, FILE *file)
{
    uint32_t readMultiSector = HALReadMultilSector(33, 1, buff, file);
    for (int i = 0; i < readMultiSector; ++i)
    {
        printf("%c", buff[i]);
    }
}