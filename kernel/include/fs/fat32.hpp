//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef FAT32_H
#define FAT32_H

#include <vector.hpp>
#include <string.hpp>
#include <pair.hpp>
#include <function.hpp>

#include <tlib/fat32_specs.hpp>

#include "disks.hpp"
#include "vfs/file_system.hpp"

namespace fat32 {

typedef const disks::disk_descriptor& dd;

struct fat32_file_system final : vfs::file_system {
    fat32_file_system(path mount_point, path device);
    ~fat32_file_system();

    void init();

    /*!
     * \copydoc vfs::file_system::statfs
     */
    size_t statfs(vfs::statfs_info& file) override;

    /*!
     * \copydoc vfs::file_system::read
     */
    size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read) override;

    /*!
     * \copydoc vfs::file_system::read
     */
    size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read, size_t ms) override;

    /*!
     * \copydoc vfs::file_system::write
     */
    size_t write(const path& file_path, const char* buffer, size_t count, size_t offset, size_t& written) override;

    /*!
     * \copydoc vfs::file_system::clear
     */
    size_t clear(const path& file_path, size_t count, size_t offset, size_t& written) override;

    /*!
     * \copydoc vfs::file_system::truncate
     */
    size_t truncate(const path& file_path, size_t size) override;

    /*!
     * \copydoc vfs::file_system::get_file
     */
    size_t get_file(const path& file_path, vfs::file& file) override;

    /*!
     * \copydoc vfs::file_system::ls
     */
    size_t ls(const path& file_path, std::vector<vfs::file>& contents) override;

    /*!
     * \copydoc vfs::file_system::touch
     */
    size_t touch(const path& file_path) override;

    /*!
     * \copydoc vfs::file_system::mkdir
     */
    size_t mkdir(const path& file_path) override;

    /*!
     * \copydoc vfs::file_system::rm
     */
    size_t rm(const path& file_path) override;

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

    path mount_point;
    path device;

    fat_bs_t* fat_bs = nullptr;
    fat_is_t* fat_is = nullptr;
};

}

#endif
