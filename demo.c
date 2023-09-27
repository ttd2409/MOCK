#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define TOTAL_SECTOR 2880
#define RootDirStartSector  19
#define RootDirSector 14
#define DATA_START_SECTOR 33
#define SPACE_CHARACTER 0x20
#define ENTRY_NAME_LENGTH 8
#define ENTRY_EXTENSION_LENGTH 3
#define ENTRY_SIZE 32
#define ATTRIBUTE_DIRECTORY 0x10
#define ENTRY_TIME_OFFSET 22
#define ENTRY_DATE_OFFSET 24
#define ENTRY_CLUSTER_OFFSET 26
#define ENTRY_FILESIZE_OFFSET 28
typedef struct 
{
    uint8_t nameOfFile[8];
    uint16_t time;
    uint16_t date;
    uint16_t clusterNumber;
    uint8_t extension[3];
    uint8_t attribute;
    uint32_t fileSize;
} Directory;
typedef struct 
{
    uint16_t year;
    uint8_t day;
    uint8_t month;
} Date;
typedef struct 
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} Time;
typedef struct 
{
   uint16_t DataStartSector;
   uint16_t DataSector;
   uint32_t Cluster;
} Data;
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
uint16_t convertLittleToBig16(uint16_t value)
{
    return (value >> 8) | (value << 8);
}
uint32_t convertLittleToBig32(uint32_t value)
{
    return ((value >> 24) & 0xFF) | ((value >> 8) & 0xFF00) | ((value << 8) & 0xFF0000) | ((value << 24) & 0xFF000000);
}
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
        boot->bytePerSector = convertLittleToBig16((buff[11] << 8) | buff[12]);
        boot->sectorsPerCluster = buff[13];
        boot->numberReservedSectors = buff[14];
        boot->FATTableNumber = buff[16];
        boot->rootEntryCount = convertLittleToBig16((buff[17] << 8) | buff[18]);
        boot->totalSector = convertLittleToBig16((buff[19] << 8) | buff[20]);
        boot->sectorPerFAT = buff[22];
    }
}
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
void readDirectory(Directory *directory, uint8_t *buff)
{
    uint32_t readMultiByte = HALReadMultilSector(RootDirStartSector, RootDirSector, buff, NULL);
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
void readData(Data * data, uint8_t *buff)
{
    data->DataStartSector = RootDirStartSector + RootDirSector;   // 33
    data->DataSector = TOTAL_SECTOR - data->DataStartSector; // 2847
    uint32_t readMultiSector = HALReadMultilSector(33, 1, buff, NULL);
    // for (int i = 0; i < readMultiSector; ++i)
    // {
    //     printf("%c", buff[i]);
    // }
}
int main()
{
    FAT fat;
    Boot boot;
    Directory directory;
    Data data1;
    uint8_t sector[512];
    uint8_t buff[ROOT_SIZE]; // root
    uint32_t cluster[2847];
    uint8_t choiceFile = 1;
    FILE *file = fopen("floppy.img", "rb");
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return -1;
    }
    readBootSector(&boot, file, sector);
    printf("FOLDER and FILES in root directory\n");
    printf("ID NAME\t\t\tSIZE\tTIME\t\tDATE\n");
    readDirectory(&directory, buff);
    // while(choiceFile != 0)
    // {
    //     printf("Data: ");
    // readData(&data1, sector, file);
    // }
    fclose(file);
    return 0;
}