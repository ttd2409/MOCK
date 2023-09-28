#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define FILE_NAME "floppy.img"
#define FILE_MODE "rb"
#define SECTOR_SIZE 512U
#define FAT_SIZE (512 * 9)
#define ROOT_SIZE (512 * 14)
#define DATA_SIZE (512 * 2847)
#define MAX_BYTE (512 * 2879)
#define SIZE_ENTRY 32U
#define CONVERT_2_BYTES(x) (((*((x) + 1)) << 8) | (*(x)))
#define CONVERT_4_BYTES(x) (((*((x) + 3)) << 24) | ((*((x) + 2)) << 16) | ((*((x) + 1)) << 8) | (*(x)))
#define TOTAL_SECTOR 2880
#define RootDirStartSector 19
#define RootDirSector 14
#define DATA_START_SECTOR 33
#define SPACE_CHARACTER 0x20
#define ENTRY_NAME_LENGTH 8
#define ENTRY_EXTENSION_LENGTH 3
#define ENTRY_SIZE 32
#define ATTRIBUTE_DIRECTORY 0x10
#define ENTRY_ATTRIBUTES_OFFSET 0x11
#define ENTRY_TIME_OFFSET 22
#define ENTRY_DATE_OFFSET 24
#define ENTRY_CLUSTER_OFFSET 26
#define ENTRY_FILESIZE_OFFSET 28

typedef struct
{
    uint8_t nameOfFile[9];
    uint16_t time;
    uint16_t date;
    uint16_t clusterNumber;
    uint8_t extension[4];
    uint8_t attribute;
    uint32_t fileSize;
    uint32_t clusterIndex;
} Directory;

typedef struct
{
    uint16_t year;
    uint8_t day;
    uint8_t month;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DateTime;

typedef struct
{
    uint16_t DataStartSector;
    uint16_t DataSector;
    uint32_t Cluster;
    uint32_t offSetData;
} Data;

typedef struct
{
    uint16_t bytePerSector;
    uint16_t totalSector;
    uint16_t sectorPerFAT;
    uint16_t rootEntryCount;
    uint8_t sectorsPerCluster;
    uint8_t numberReservedSectors;
    uint8_t FATTableNumber;
} Boot;

typedef struct
{
    uint8_t fatStartSector;
    uint8_t fatSector;
} FAT;

uint32_t HALReadSector(uint32_t index, uint8_t *buff)
{
    static FILE *file = NULL;
    if (file == NULL)
    {
        file = fopen(FILE_NAME, FILE_MODE);
        if (file == NULL)
        {
            printf("Cannot open file\n");
            return -1;
        }
    }
    fseek(file, index * SECTOR_SIZE, SEEK_SET);
    uint32_t byteRead = fread(buff, sizeof(uint8_t), SECTOR_SIZE, file);
    return byteRead;
}

uint32_t HALReadMultilSector(uint32_t index, uint32_t num, uint8_t *buff)
{
    static FILE *file = NULL;
    if (file == NULL)
    {
        file = fopen(FILE_NAME, FILE_MODE);
        if (file == NULL)
        {
            printf("Cannot open file\n");
            return -1;
        }
    }
    uint32_t sectorAddress = index * SECTOR_SIZE;
    fseek(file, sectorAddress, SEEK_SET);
    uint32_t byteRead = 0;
    for (uint32_t i = 0; i < num; i++)
    {
        uint32_t sectorRead = fread(buff + (i * SECTOR_SIZE), sizeof(uint8_t), SECTOR_SIZE, file);
        byteRead += sectorRead;
    }
    return byteRead;
}

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

void convertEntryToDirectory(Directory *directory, uint8_t *buff, uint32_t index)
{
    memcpy(directory->nameOfFile, &buff[index], ENTRY_NAME_LENGTH);
    directory->nameOfFile[ENTRY_NAME_LENGTH] = '\0';
    memcpy(directory->extension, &buff[index + ENTRY_NAME_LENGTH + 1], ENTRY_EXTENSION_LENGTH);
    directory->extension[ENTRY_EXTENSION_LENGTH] = '\0';
    directory->time = CONVERT_2_BYTES(&buff[index + ENTRY_TIME_OFFSET]);
    directory->date = CONVERT_2_BYTES(&buff[index + ENTRY_DATE_OFFSET]);
    directory->clusterNumber = CONVERT_2_BYTES(&buff[index + ENTRY_CLUSTER_OFFSET]);
    directory->fileSize = CONVERT_4_BYTES(&buff[index + ENTRY_FILESIZE_OFFSET]);
}

void printDirectory(Directory *directory, uint8_t index)
{
    DateTime result = convertDateTime(directory->date, directory->time);
    printf("%-2d: %-8s %-4s %10d ", index, directory->nameOfFile, directory->extension, directory->fileSize);
    printf("\t%02d/%02d/%04d\t%02d:%02d:%02d %x\n", result.day, result.month, result.year, result.hour, result.minute, result.second, directory->clusterNumber);
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

void readFAT(uint8_t *buff, uint32_t *fat)
{
    uint32_t fatStartSector = RootDirStartSector + RootDirSector;
    uint32_t fatSector = HALReadMultilSector(fatStartSector, FAT_SIZE, buff);
    uint32_t fatEntryCount = fatSector * SECTOR_SIZE / 3 * 4;
    uint32_t fatIndex = 0;
    for (uint32_t i = 0; i < fatEntryCount; i += 4)
    {
        fat[fatIndex] = CONVERT_4_BYTES(&buff[i]) & 0x0FFFFFFF;
        fatIndex++;
    }
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
            printDirectory(directory, index);
            index++;
        }
    }
}

void readSubDirectory(Directory *directory, uint8_t *buff, uint32_t *fat, uint32_t clusterIndex)
{
    uint32_t dataStartSector = RootDirStartSector + RootDirSector;
    uint32_t dataSector = TOTAL_SECTOR - dataStartSector;
    uint32_t offsetData = DATA_START_SECTOR + clusterIndex - 2;
    uint32_t readSector = HALReadSector(offsetData, buff);
    uint32_t maxEntries = readSector * SECTOR_SIZE / SIZE_ENTRY;
    uint32_t index = 1;
    for (uint32_t j = 0; j < maxEntries; j++)
    {
        uint32_t entryIndex = j * SIZE_ENTRY;
        if (buff[entryIndex] == 0)
        {
            return;
        }
        else if (buff[entryIndex + ENTRY_ATTRIBUTES_OFFSET] != 0x0F)
        {
            convertEntryToDirectory(&directory[index], buff, entryIndex);
            directory[index].clusterIndex = clusterIndex;
            printDirectory(&directory[index], index);
            index++;
        }
        else if (buff[entryIndex] == 0xE5)
        {
            continue;
        }
        else if (buff[entryIndex + ENTRY_ATTRIBUTES_OFFSET] == ATTRIBUTE_DIRECTORY)
        {
            uint32_t subCluster = CONVERT_2_BYTES(&buff[entryIndex + ENTRY_CLUSTER_OFFSET]);
            if (subCluster >= 0x0002 && subCluster <= 0x0FEF)
            {
                readSubDirectory(directory, buff, fat, subCluster);
            }
            else if (subCluster >= 0x0FF0)
            {
                // Long File Name Entry, ignore
            }
            else if (subCluster == 0x0000 || subCluster >= 0x0FF7)
            {
                // End of Cluster Chain or Reserved Cluster, ignore
            }
            else
            {
                uint32_t nextCluster = fat[subCluster];
                while (nextCluster >= 0x0002 && nextCluster <= 0x0FEF)
                {
                    readSubDirectory(directory, buff, fat, nextCluster);
                    nextCluster = fat[nextCluster];
                }
            }
        }
    }
}

uint32_t readData(Data *data, uint8_t *buff, uint32_t cluster)
{
    uint32_t dataStartSector = RootDirStartSector + RootDirSector;
    uint32_t dataSector = TOTAL_SECTOR - dataStartSector;
    uint32_t offsetData = DATA_START_SECTOR + cluster - 2;
    uint32_t readSector = HALReadMultilSector(offsetData, 1, buff);
    // Process the data here
    return readSector;
}

int main()
{
    FAT fat1;
    Boot boot;
    Data data1;
    Directory directory[ROOT_SIZE / ENTRY_SIZE];
    uint8_t buff[MAX_BYTE];
    uint32_t fat[FAT_SIZE / 4];
    uint32_t clusterIndex = 0;
    int8_t choiceFile = 1;
    readBootSector(&boot, buff);
    readFAT(buff, fat);
    printf("FOLDER và FILE trong thư mục gốc\n");
    printf("ID TÊN\t\tLOẠI\t KÍCH THƯỚC\t\tNGÀY\t\tGIỜ\n");
    readSubDirectory(directory, buff, fat, clusterIndex);
    while (choiceFile != -1)
    {
        printf("\nChọn file hoặc thư mục để mở: ");
        scanf("%d", &choiceFile);
        printf("\n");
        if (choiceFile != 0 && choiceFile < clusterIndex)
        {
            if (directory[choiceFile].attribute != ATTRIBUTE_DIRECTORY)
            {
                uint32_t cluster = directory[choiceFile].clusterNumber;
                while (cluster >= 0x0002 && cluster <= 0x0FEF)
                {
                    uint32_t read = readData(&data1, buff, cluster);
                    for (int i = 0; i < read; ++i)
                    {
                        printf("%c", buff[i]);
                    }
                    cluster = fat[cluster];
                }
                printf("\n");
            }
        }
        else if (directory[choiceFile].clusterNumber == 0)
        {
            clusterIndex++;
            readSubDirectory(directory, buff, fat, clusterIndex);
        }
        else
        {
        }
    }
    return 0;
}