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
    FILE *file = fopen("floppy.img", "rb");
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return -1;
    }
    readBootSector(&boot, file, sector);    
    printf("FOLDER and FILES in root directory\n");
    printf("ID NAME\t\t\tSIZE\tDATE\t\tTIME\n");
    // while(choiceFile != 0)
    // {

    readDirectory(&directory, file, buff,cluster,attribute);
    while (choiceFile != -1)
    {
        printf("\nChoose file or folder to open: ");
        scanf("%d", &choiceFile);
        printf("\n");
        if(choiceFile != 0 && choiceFile < directory.clusterIndex)
        {
            if(attribute[choiceFile] != 0x10)
            {
                readData(&data1, buff, file, cluster[choiceFile]);
                printf("\n");
            }
        }
        else if(cluster[choiceFile] == 0)
        {
            readDirectory(&directory, file, buff,cluster,attribute);
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

    fclose(file);
    return 0;
}
