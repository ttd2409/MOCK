#include "read_file.h"
#include "read_file.c"
#include "HAL.h"
#include "HAL.c"

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

    //     for (int a = 0; a < 10; a++)
    // {
    //         printf("%x ", attribute[a]);
    // }
    // printf("\n");
    //     for (int a = 0; a < directory.clusterIndex; a++)
    // {
    //         printf("%d ", cluster[a]);
    // }
    //     printf("Data: ");
    // readData(&data1, sector, file);
    // }

    return 0;
}
