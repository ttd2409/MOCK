#include "read_file.h"
#include "read_file.c"
#include "HAL.h"
#include "HAL.c"

int main()
{
    FAT fat;
    Boot boot;
    Directory directory;
    uint32_t numberSector;
    uint32_t startSector;
    uint8_t sector[512];
    uint8_t buff[ROOT_SIZE]; // root
    FILE *file = fopen("floppy.img", "rb");
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return -1;
    }
    uint8_t choice;
            readBootSector(&boot, file, sector);
            printf("FOLDER and FILES in root directory\n");
            printf("ID NAME\t\t\tSIZE\tTIME\t\tDATE\n");
            readDirectory(&directory, file, buff);

            printf("Data: ");
            readData(sector, file);
    // printf("**************************************************************\n");
    // printf("**  1 - Display all folders and files in the root directory **\n");
    // printf("**  2 - Displaying subfolders and files within a folder     **\n");
    // printf("**  3 - Display the content of a selected file on the screen**\n");
    // printf("**  4 - Exit the program                                    **\n");
    // printf("**************************************************************\n");
    // do
    // {
    //     printf("Enter your choice:  ");
    //     scanf("%d", &choice);
    //     switch (choice)
    //     {
    //     case 1:
    //         break;
    //     case 2:
    //         break;
    //     case 3:
    //         break;
    //     case 4:
        
    //         break;
    //     default:
    //         break;
    //     }
    // } while (choice != 4);
    fclose(file);
    return 0;
}
