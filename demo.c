#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define SECTOR_SIZE 512U
#define FAT_SIZE (512 * 9)
#define ROOT_SIZE (512 * 14)
#define DATA_SIZE (512 * 2847)
#define MAX_BYTE (512 * 2879)

typedef struct BOOT_SECTOR
{
    uint16_t bytePerSector;
    uint8_t sectorsPerCluster;
    uint8_t numberReservedSectors;
    uint8_t FATTableNumber;
    uint16_t rootEntryCount;
    uint16_t totalSector;
    uint16_t sectorPerFAT;
} Boot;

typedef struct FAT
{
    uint8_t fatStartSector;
    uint8_t fatSector;
} FAT;

typedef struct Directory
{
    uint8_t RootDirStartSector;
    uint8_t RootDirSector;
    uint8_t nameOfFile[8];
    uint8_t extension[3];
    uint8_t attribute;
    uint16_t time;
    uint16_t dateMonthYear;
    uint16_t clusterNumber;
    uint32_t fileSize;
} Directory;

uint32_t HALReadSector(uint32_t index, uint8_t *buff, FILE *file);
uint32_t HALReadMultilSector(uint32_t index, uint32_t num, uint8_t *buff, FILE *file);
void readBootSector(Boot *boot, FILE *fp, uint8_t *buff);
void readDirectory(Directory *directory, Boot *boot, FILE *fp, uint8_t *buff);
void printDirectoryInfo(Directory *directory);
static uint16_t convertLittleToBig(uint16_t value);

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
    fseek(fp, sectorAddress, SEEK_SET);
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
        boot->bytePerSector = convertLittleToBig((buff[11] << 8) | buff[12]);
        boot->sectorsPerCluster = buff[13];
        boot->numberReservedSectors = buff[14];
        boot->FATTableNumber = buff[16];
        boot->rootEntryCount = convertLittleToBig((buff[17] << 8) | buff[18]);
        boot->totalSector = convertLittleToBig((buff[19] << 8) | buff[20]);
        boot->sectorPerFAT = buff[22];
    }
}

void readDirectory(Directory *directory, Boot *boot, FILE *fp, uint8_t *buff)
{
    directory->RootDirStartSector = boot->numberReservedSectors + (boot->FATTableNumber * boot->sectorPerFAT);
    directory->RootDirSector = (32 * boot->rootEntryCount + boot->bytePerSector - 1) / boot->bytePerSector;
    uint32_t readMultiByte = HALReadMultilSector(directory->RootDirStartSector, directory->RootDirSector, buff, fp);
    for (uint32_t i = 0; i < directory->RootDirSector; i++)
    {
        for (uint32_t j = 0; j < boot->bytePerSector; j += 32)
        {
            uint32_t entryStartByte = i * boot->bytePerSector + j;
            if (buff[entryStartByte] == 0)
            {
                return;
            }
            bool isDirectory = (buff[entryStartByte + 11] == 0x10);
            for (uint8_t index = 0; index < 8; index++)
            {
                directory->nameOfFile[index] = buff[entryStartByte + index];
            }
            directory->nameOfFile[8] = '\0';
            if (!isDirectory)
            {
                for (uint8_t index = 0; index < 3; index++)
                {
                    directory->extension[index] = buff[entryStartByte + 8 + index];
                }
                directory->extension[3] = '\0';
                uint32_t fileSize = convertLittleToBig((buff[entryStartByte + 28] << 24) | (buff[entryStartByte + 29] << 16) | (buff[entryStartByte + 30] << 8) | buff[entryStartByte + 31]);
                directory->fileSize = fileSize;
                uint16_t clusterNumber = convertLittleToBig((buff[entryStartByte + 20] << 8) | buff[entryStartByte + 21]);
                directory->clusterNumber = clusterNumber;
            }
            printDirectoryInfo(directory);
        }
    }
}

void printDirectoryInfo(Directory *directory)
{
    printf("%s\n", directory->nameOfFile);
}

static uint16_t convertLittleToBig(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

int main()
{
    FAT fat;
    Boot boot;
    Directory directory;
    uint32_t numberSector;
    uint32_t startSector;
    uint8_t sector[512];
    uint8_t buff[ROOT_SIZE];
    FILE *file = fopen("floppy.img", "rb");
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return -1;
    }
    uint32_t byteReadSector = HALReadSector(0, sector, file);
    readBootSector(&boot, file, sector);
    readDirectory(&directory, &boot, file, buff);
    fclose(file);
    return 0;
}