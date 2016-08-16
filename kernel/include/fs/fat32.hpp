//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FAT32_H
#define FAT32_H

#include <vector.hpp>
#include <string.hpp>
#include <pair.hpp>
#include <function.hpp>

#include "disks.hpp"
#include "vfs/file_system.hpp"
#include "fs/fat32_specs.hpp"

namespace fat32 {

typedef const disks::disk_descriptor& dd;

struct fat32_file_system : vfs::file_system {
private:
    path mount_point;
    std::string device;

    fat_bs_t* fat_bs = nullptr;
    fat_is_t* fat_is = nullptr;

public:
    fat32_file_system(path mount_point, std::string device);
    ~fat32_file_system();

    void init();

    size_t statfs(statfs_info& file);
    size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read);
    size_t write(const path& file_path, const char* buffer, size_t count, size_t offset, size_t& written);
    size_t clear(const path& file_path, size_t count, size_t offset, size_t& written);
    size_t truncate(const path& file_path, size_t size);
    size_t get_file(const path& file_path, vfs::file& file);
    size_t ls(const path& file_path, std::vector<vfs::file>& contents);
    size_t touch(const path& file_path);
    size_t mkdir(const path& file_path);
    size_t rm(const path& file_path);

private:
    size_t rm_dir(uint32_t parent_cluster_number, size_t position, uint32_t cluster_number);
    size_t rm_file(uint32_t parent_cluster_number, size_t position, uint32_t cluster_number);

    size_t change_directory_entry(uint32_t parent_cluster_number, size_t position, const std::function<void(cluster_entry&)>& functor);

    cluster_entry* find_free_entry(std::unique_heap_array<cluster_entry>& directory_cluster, size_t entries, uint32_t& cluster_number);
    cluster_entry* extend_directory(std::unique_heap_array<cluster_entry>& directory_cluster, size_t entries, uint32_t& cluster_number);

    std::vector<vfs::file> files(const path& path, size_t last = 0);
    std::pair<bool, uint32_t> find_cluster_number(const path& path, size_t last = 0);
    std::vector<vfs::file> files(uint32_t cluster_number);

    bool write_is();
    uint64_t cluster_lba(uint64_t cluster);
    uint32_t read_fat_value(uint32_t cluster);
    bool write_fat_value(uint32_t cluster, uint32_t value);
    uint32_t next_cluster(uint32_t cluster);
    uint32_t find_free_cluster();

    bool read_sectors(uint64_t start, uint8_t count, void* destination);
    bool write_sectors(uint64_t start, uint8_t count, void* source);
};

}

#endif
