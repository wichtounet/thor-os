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
#include <pair.hpp>

#include "disks.hpp"
#include "vfs/file_system.hpp"
#include "fs/fat32_specs.hpp"

namespace fat32 {

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

    size_t statfs(statfs_info& file);
    size_t read(const std::vector<std::string>& file_path, char* buffer, size_t count, size_t offset, size_t& read);
    size_t write(const std::vector<std::string>& file_path, char* buffer, size_t count, size_t offset, size_t& written);
    size_t get_file(const std::vector<std::string>& file_path, vfs::file& file);
    size_t ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents);
    size_t touch(const std::vector<std::string>& file_path);
    size_t mkdir(const std::vector<std::string>& file_path);
    size_t rm(const std::vector<std::string>& file_path);

private:
    size_t rm_dir(uint32_t parent_cluster_number, size_t position, uint32_t cluster_number);
    size_t rm_file(uint32_t parent_cluster_number, size_t position, uint32_t cluster_number);

    cluster_entry* find_free_entry(std::unique_heap_array<cluster_entry>& directory_cluster, size_t entries, uint32_t& cluster_number);
    cluster_entry* extend_directory(std::unique_heap_array<cluster_entry>& directory_cluster, size_t entries, uint32_t& cluster_number);

    std::vector<vfs::file> files(const std::vector<std::string>& path, size_t last = 0);
    std::pair<bool, uint32_t> find_cluster_number(const std::vector<std::string>& path, size_t last = 0);
    std::vector<vfs::file> files(uint32_t cluster_number);

    bool write_is();
    uint64_t cluster_lba(uint64_t cluster);
    uint32_t read_fat_value(uint32_t cluster);
    bool write_fat_value(uint32_t cluster, uint32_t value);
    uint32_t next_cluster(uint32_t cluster);
    uint32_t find_free_cluster();
};

}

#endif
