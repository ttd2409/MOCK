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
    uint8_t nameOfFile[8];
    uint16_t time;
    uint16_t date;
    uint16_t clusterNumber;
    uint8_t extension[3];
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

void readBootSector(Boot *boot, uint8_t *buff);

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
            printDirectory(directory, index);
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
int main()
{
    FAT fat;
    Boot boot;
    Directory directory;
    Data data1;
    uint8_t sector[512];
    uint8_t buff[ROOT_SIZE]; // root
    uint32_t cluster[224];
    uint32_t attribute[224];
    int8_t choiceFile = 1;

    readBootSector(&boot, sector);
    printf("FOLDER and FILES in root directory\n");
    printf("ID NAME\t\tTYPE\t  SIZE\t\tDATE\t\tTIME\n");
    readDirectory(&directory, buff, cluster, attribute);

    while (choiceFile != -1)
    {
        printf("\nChoose file or folder to open: ");
        scanf("%d", &choiceFile);
        printf("\n");
        if (choiceFile != 0 && choiceFile < directory.clusterIndex)
        {
            if (attribute[choiceFile] != 0x10)
            {
                uint32_t read = readData(&data1, buff, cluster[choiceFile]);
                for (int i = 0; i < read; ++i)
                {
                    printf("%c", buff[i]);
                }
                printf("\n");
            }
        }
        else if (cluster[choiceFile] == 0)
        {
            readDirectory(&directory, buff, cluster, attribute);
        }
        else
        {
        }
    }
    return 0;
}
