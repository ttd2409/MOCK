#include "read_file.h"
// #include "read_file.c"
#include "HAL.h"
// #include "HAL.c"

int main()
{

    Boot boot;
    Directory directory;
    Data data1;
    uint8_t *sector = NULL;
    uint8_t buff[DATA_SIZE];
    uint32_t cluster[2847];
    uint32_t attribute[2847];
    int8_t choiceFile = 1;

    sector = (uint8_t *)malloc(SECTOR_SIZE);
    readBootSector(&boot);
    free(sector);

    printf("FOLDER and FILES in root directory\n");
    printf("ID NAME\t\tTYPE\t  SIZE\t\tDATE\t\tTIME\n");
    readRootDirectory(&directory, boot, cluster, attribute);
    do
    {
        printf("\nChoose file or folder to open: ");
        scanf("%d", &choiceFile);
        printf("\n");
        if (choiceFile != 0 && choiceFile <= directory.clusterIndex)
        {
            if (attribute[choiceFile] != 0x10)
            {
                // FATShowData(&boot, sector, cluster[choiceFile]);
                readData(&boot, cluster[choiceFile]);
            }
            else
            {
                readSub(&directory, boot, cluster[choiceFile], attribute, cluster);
            }
        }
        else if (choiceFile == 0)
        {
            readRootDirectory(&directory, boot, cluster, attribute);
        }
        else if (choiceFile == -1)
        {
            printf("\nQuit program, thank for using.\n");
        }
        else if (choiceFile > directory.clusterIndex)
        {
            printf("Error\n");
        }
    } while (choiceFile != -1);

    return 0;
}
