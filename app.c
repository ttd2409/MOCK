#include "read_file.h"
#include "read_file.c"
#include "HAL.h"
#include "HAL.c"
void FATShowData(Data *data1, uint8_t *buff, uint32_t cluster)
{
    uint32_t read = readData(data1, buff, cluster);
    for (int i = 0; i < read; ++i)
    {
        printf("%c", buff[i]);
    }
    printf("\n");
}
int main()
{

    Boot boot;
    Directory directory;
    Data data1;
    uint8_t sector[512];
    uint8_t buff[ROOT_SIZE]; // root
    uint8_t * buff1 = (uint8_t*)malloc(512);
    uint32_t cluster[2847];
    uint32_t attribute[2847];
    int8_t choiceFile = 1;

    readBootSector(&boot, sector);
    printf("FOLDER and FILES in root directory\n");
    printf("ID NAME\t\tTYPE\t  SIZE\t\tDATE\t\tTIME\n");
    readRootDirectory(&directory, buff, cluster, attribute);
    // for (int i = 0; i < 512; i++)
    // {

    //     uint32_t a = readFAT(buff, 1);
    //     printf("%x", a);
    // }
    do
    {
        printf("\nChoose file or folder to open: ");
        scanf("%d", &choiceFile);
        printf("\n");
        if (choiceFile != 0 && choiceFile < directory.clusterIndex)
        {
            if (attribute[choiceFile] != 0x10)
            {
                FATShowData(&data1, sector, cluster[choiceFile]);
            }
            else if (cluster[choiceFile] == 0)
            {
                readRootDirectory(&directory, buff, cluster, attribute);
            }
            else
            {
                readSubDirectory(&directory, buff, cluster[choiceFile], attribute);
            }
        }
        else if (choiceFile == -1)
        {
            printf("\nQuit program, thank for using.\n");
        }
        else if(choiceFile > directory.clusterIndex)
        {
            printf("Error\n");
        }
    } while (choiceFile != -1);

    return 0;
}
