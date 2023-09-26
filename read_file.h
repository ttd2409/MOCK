#ifndef _READ_FILE_H_
#define _READ_FILE_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define RootDirStartSector  19
#define RootDirSector 14
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
}Date;

typedef struct 
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}Time;

void readDirectory(Directory * directory, FILE *fp, uint8_t *buff);
void printDirectory(Directory *directory, bool isFolder);
void readFAT(uint8_t *buff, FILE * file);
void readData(uint8_t *buff, FILE * file);

#endif    /*  */