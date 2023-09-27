#include <stdio.h>
#include <stdint.h>

// Define some constants for convenience
#define BOOT_SECTOR_SIZE 512
#define RDET_ENTRY_SIZE 32
#define END_OF_CHAIN 0xFFF

// Define a struct to store the boot sector information
typedef struct {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
} boot_sector_t;

// Define a struct to store the RDET entry information
typedef struct {
    char name[11];
    uint8_t attributes;
    uint8_t reserved[10];
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster;
    uint32_t file_size;
} rdet_entry_t;

// Define a function to read the boot sector from a file and store it in a struct
void read_boot_sector(FILE *fp, boot_sector_t *bs) {
    // Seek to the beginning of the file
    fseek(fp, 0, SEEK_SET);
    // Read the bytes per sector (offset 11, size 2)
    fread(&bs->bytes_per_sector, 2, 1, fp);
    // Read the sectors per cluster (offset 13, size 1)
    fread(&bs->sectors_per_cluster, 1, 1, fp);
    // Read the reserved sectors (offset 14, size 2)
    fread(&bs->reserved_sectors, 2, 1, fp);
    // Read the number of FATs (offset 16, size 1)
    fread(&bs->number_of_fats, 1, 1, fp);
    // Read the root directory entries (offset 17, size 2)
    fread(&bs->root_dir_entries, 2, 1, fp);
    // Read the total sectors (offset 19, size 2)
    fread(&bs->total_sectors, 2, 1, fp);
    // Read the media descriptor (offset 21, size 1)
    fread(&bs->media_descriptor, 1, 1, fp);
    // Read the sectors per FAT (offset 22, size 2)
    fread(&bs->sectors_per_fat, 2, 1, fp);
}

// Define a function to read a RDET entry from a file and store it in a struct
void read_rdet_entry(FILE *fp, rdet_entry_t *re) {
    // Read the name (offset 0, size 11)
    fread(re->name, 11, 1, fp);
    // Read the attributes (offset 11, size 1)
    fread(&re->attributes, 1, 1, fp);
    // Read the reserved bytes (offset 12, size 10)
    fread(re->reserved, 10, 1 ,fp);
    // Read the write time (offset 22, size 2)
    fread(&re->write_time, 2 ,1 ,fp);
    // Read the write date (offset 24 ,size ,2)
    fread(&re->write_date ,2 ,1 ,fp);
    // Read the first cluster (offset ,26 ,size ,2)
    fread(&re->first_cluster ,2 ,1 ,fp);
    // Read the file size (offset ,28 ,size ,4)
    fread(&re->file_size ,4 ,1 ,fp);
}

// Define a function to print the boot sector information
void print_boot_sector(boot_sector_t *bs) {
    printf("Boot sector information:\n");
    printf("Bytes per sector: %u\n", bs->bytes_per_sector);
    printf("Sectors per cluster: %u\n", bs->sectors_per_cluster);
    printf("Reserved sectors: %u\n", bs->reserved_sectors);
    printf("Number of FATs: %u\n", bs->number_of_fats);
    printf("Root directory entries: %u\n", bs->root_dir_entries);
    printf("Total sectors: %u\n", bs->total_sectors);
    printf("Media descriptor: %02X\n", bs->media_descriptor);
    printf("Sectors per FAT: %u\n", bs->sectors_per_fat);
}

// Define a function to print the RDET entry information
void print_rdet_entry(rdet_entry_t *re) {
    printf("RDET entry information:\n");
    printf("Name: %s\n", re->name);
    printf("Attributes: %02X\n", re->attributes);
    printf("Write time: %04X\n", re->write_time);
    printf("Write date: %04X\n", re->write_date);
    printf("First cluster: %u\n", re->first_cluster);
    printf("File size: %u\n", re->file_size);
}

// Define a function to read the FAT from a file and store it in an array
void read_fat(FILE *fp, boot_sector_t *bs, uint16_t *fat) {
    // Seek to the start of the first FAT (after the reserved sectors)
    fseek(fp, bs->reserved_sectors * bs->bytes_per_sector, SEEK_SET);
    // Read the FAT entries (each entry is 12 bits, or 1.5 bytes)
    for (int i = 0; i < bs->sectors_per_fat * bs->bytes_per_sector / 3 * 2; i++) {
        // Read two bytes (16 bits) from the file
        uint16_t bytes;
        fread(&bytes, 2, 1, fp);
        // Split the bytes into two entries (12 bits each)
        uint16_t entry1 = bytes & 0x0FFF; // Mask the lower 12 bits
        uint16_t entry2 = bytes >> 4; // Shift the upper 12 bits to the right
        // Store the entries in the array
        fat[i] = entry1;
        fat[i+1] = entry2;
        // Increment i by one to skip the next entry
        i++;
    }
}

// Define a function to print the FAT information
void print_fat(boot_sector_t *bs, uint16_t *fat) {
    printf("FAT information:\n");
    // Loop through the FAT entries
    for (int i = 0; i < bs->sectors_per_fat * bs->bytes_per_sector / 3 * 2; i++) {
        // Print the entry value in hexadecimal format
        printf("%03X ", fat[i]);
        // Print a new line every 16 entries for readability
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
}

// Define a function to get the next cluster in the chain given a current cluster
uint16_t get_next_cluster(uint16_t current_cluster, uint16_t *fat) {
    // Return the value of the FAT entry at the index of the current cluster
    return fat[current_cluster];
}

// Define a function to print the cluster chain of a file given its first cluster
void print_cluster_chain(uint16_t first_cluster, uint16_t *fat) {
    printf("Cluster chain:\n");
    // Initialize a variable to store the current cluster
    uint16_t current_cluster = first_cluster;
    // Loop until the end of the chain is reached
    while (current_cluster != END_OF_CHAIN) {
        // Print the current cluster in hexadecimal format
        printf("%03X ", current_cluster);
        // Get the next cluster from the FAT
        current_cluster = get_next_cluster(current_cluster, fat);
    }
    // Print a new line at the end
    printf("\n");
}

// Define a function to read and print all RDET entries from a file
void read_and_print_all_rdet_entries(FILE *fp, boot_sector_t *bs, uint16_t *fat) {
    // Seek to the start of the root directory (after the reserved sectors and FATs)
    fseek(fp, (bs->reserved_sectors + bs->number_of_fats * bs->sectors_per_fat) * bs->bytes_per_sector, SEEK_SET);
    // Loop through all RDET entries
    for (int i = 0; i < bs->root_dir_entries; i++) {
        // Read a RDET entry from the file and store it in a struct
        rdet_entry_t re;
        read_rdet_entry(fp, &re);
        // Check if the entry is valid (the first byte of the name is not 0x00 or 0xE5)
        if (re.name[0] != 0x00 && re.name[0] != 0xE5) {
            // Print the RDET entry information
            print_rdet_entry(&re);
            // Print the cluster chain of the file
            print_cluster_chain(re.first_cluster, fat);
            // Print a separator line
            printf("--------------------------------------\n");
        }
    }
}
int main() {
    // Open the file
    FILE *fp = fopen("floppy.img", "rb");
    if (fp == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // Read the boot sector
    boot_sector_t bs;
    read_boot_sector(fp, &bs);

    // Print the boot sector information
    print_boot_sector(&bs);

    // Read the FAT
    uint16_t fat[bs.sectors_per_fat * bs.bytes_per_sector / 3 * 2];
    read_fat(fp, &bs, fat);

    // Print the FAT information
    print_fat(&bs, fat);

    // Read and print all RDET entries
    read_and_print_all_rdet_entries(fp, &bs, fat);

    // Close the file
    fclose(fp);

    return 0;
}
