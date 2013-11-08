#include "unique_ptr.hpp"

#include "fat32.hpp"
#include "types.hpp"
#include "console.hpp"

namespace {

//FAT 32 Boot Sector
struct fat_bs_t {
    uint8_t jump[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fat;
    uint16_t root_directories_entries;
    uint16_t total_sectors;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_long;
    uint32_t sectors_per_fat_long;
    uint16_t drive_description;
    uint16_t version;
    uint32_t root_directory_cluster_start;
    uint16_t fs_information_sector;
    uint16_t boot_sectors_copy_sector;
    uint8_t filler[12];
    uint8_t physical_drive_number;
    uint8_t reserved;
    uint8_t extended_boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char file_system_type[8];
    uint8_t boot_code[420];
    uint16_t signature;
}__attribute__ ((packed));

struct fat_is_t {
    uint32_t signature_start;
    uint8_t reserved[480];
    uint32_t signature_middle;
    uint32_t free_clusters;
    uint32_t allocated_clusters;
    uint8_t reserved_2[12];
    uint32_t signature_end;
}__attribute__ ((packed));

//static_assert(sizeof(fat_bs_t) == 512, "FAT Boot Sector is exactly one disk sector");

} //end of anonymous namespace

void fat32::ls(const disks::disk_descriptor& disk, const disks::partition_descriptor& partition){
    unique_ptr<uint64_t, malloc_delete<uint64_t>> fat_bs_buffer(k_malloc(512));

    if(!read_sectors(disk, partition.start, 1, fat_bs_buffer.get())){
        //TODO
    } else {
        auto* fat_bs = reinterpret_cast<fat_bs_t*>(fat_bs_buffer.get());

        //fat_bs->signature should be 0xAA55
        //fat_bs->file_system_type should be FAT32

        auto fs_information_sector = partition.start + static_cast<uint64_t>(fat_bs->fs_information_sector);

        unique_ptr<uint64_t, malloc_delete<uint64_t>> fat_is_buffer(k_malloc(512));

        if(!read_sectors(disk, fs_information_sector, 1, fat_is_buffer.get())){
            //TODO
        } else {
            auto* fat_is = reinterpret_cast<fat_is_t*>(fat_is_buffer.get());

            //fat_is->signature_start should be 0x52 0x52 0x61 0x41
            //fat_is->signature_middle should be 0x72 0x72 0x41 0x61
            //fat_is->signature_end should be 0x00 0x00 0x55 0xAA
        }
    }
}
