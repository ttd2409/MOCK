#ifndef _READ_FILE_H_
#define _READ_FILE_H_

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
}DateTime;

typedef struct 
{
   uint16_t DataStartSector;
   uint16_t DataSector;
   uint32_t Cluster;
   uint32_t offSetData;
}Data;


void readDirectory(Directory *directory, uint8_t *buff, uint32_t *cluster,uint32_t * attribute);
// void printDirectory(Directory *directory, bool isFolder, uint8_t index);
void readSubDirectory(Directory * directory, uint32_t *cluster);
void readFAT(uint8_t *buff);
uint32_t readData(Data *data, uint8_t *buff,uint32_t cluster);

#endif    /*  */