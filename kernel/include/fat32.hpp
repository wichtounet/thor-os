//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FAT32_H
#define FAT32_H

#include <vector.hpp>
#include <string.hpp>

#include "disks.hpp"
#include "file_system.hpp"

namespace fat32 {

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

//FAT 32 Information sector
struct fat_is_t {
    uint32_t signature_start;
    uint8_t reserved[480];
    uint32_t signature_middle;
    uint32_t free_clusters;
    uint32_t allocated_clusters;
    uint8_t reserved_2[12];
    uint32_t signature_end;
}__attribute__ ((packed));

static_assert(sizeof(fat_bs_t) == 512, "FAT Boot Sector is exactly one disk sector");

typedef const disks::disk_descriptor& dd;

struct fat32_file_system : vfs::file_system {
private:
    dd disk;
    disks::partition_descriptor partition;

    fat_bs_t* fat_bs = nullptr;
    fat_is_t* fat_is = nullptr;
    uint64_t partition_start = 0;

public:
    fat32_file_system(size_t disk, size_t partition);
    ~fat32_file_system();

    size_t read(const std::vector<std::string>& file_path, std::string& content);
    size_t ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents);
    size_t touch(const std::vector<std::string>& file_path);
    size_t mkdir(const std::vector<std::string>& file_path);
    size_t rm(const std::vector<std::string>& file_path);
};

uint64_t free_size(dd disk, const disks::partition_descriptor& partition);
std::vector<vfs::file> ls(dd disk, const disks::partition_descriptor& partition, const std::vector<std::string>& path);
std::string read_file(dd disk, const disks::partition_descriptor& partition, const std::vector<std::string>& path, const std::string& file);
bool mkdir(dd disk, const disks::partition_descriptor& partition, const std::vector<std::string>& path, const std::string& directory);
bool touch(dd disk, const disks::partition_descriptor& partition, const std::vector<std::string>& path, const std::string& file);
bool rm(dd disk, const disks::partition_descriptor& partition, const std::vector<std::string>& path, const std::string& file);

}

#endif
