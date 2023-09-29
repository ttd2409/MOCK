#ifndef _READ_FILE_H_
#define _READ_FILE_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define TOTAL_SECTOR 2880
// #define RootDirStartSector  19
// #define RootDirSector 14
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
    uint16_t year;
    uint8_t day;
    uint8_t month;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} Directory;

typedef struct 
{
   uint16_t DataStartSector;
   uint16_t DataSector;
   uint32_t Cluster;
   uint32_t offSetData;
}Data;
typedef struct 
{
    uint16_t bytePerSector;
    uint16_t totalSector;
    uint16_t sectorPerFAT;
    uint16_t rootEntryCount;
    uint8_t sectorsPerCluster;
    uint8_t numberReservedSectors;
    uint8_t FATTableNumber;
    uint8_t FATStartSector;
    uint8_t FAtSector;
    uint16_t RootDirStartSector;
    uint16_t RootDirSector;
    uint16_t DataStartSector;
    uint16_t DataSector;
    uint16_t countCluster;
} Boot;
void readBootSector(Boot *boot);
void readRootDirectory(Directory *directory,Boot boot, uint32_t *cluster, uint32_t *attribute);
void printDirectory(Directory *directory,uint8_t index);
void readSub(Directory *directory, Boot boot, uint32_t first, uint32_t *attribute, uint32_t *cluster);
void readData(Boot *boot, uint32_t cluster);
// void FATShowData(Boot * boot, uint8_t *buff, uint32_t cluster);
#endif    /*  */