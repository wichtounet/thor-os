//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <unique_ptr.hpp>
#include <math.hpp>
#include <array.hpp>

#include <tlib/file.hpp>
#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/io.hpp>
#include <tlib/print.hpp>
#include <tlib/fat32_specs.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

int main(int argc, char* argv[]){
    if(argc < 3){
        tlib::printf("usage: mkfs fs device \n");

        return 1;
    }

    auto fs_str = argv[1];
    auto device_str = argv[2];

    std::string fs(fs_str);

    if(fs == "fat32"){
        // Open the device file
        auto fd = tlib::open(device_str);

        if(!fd.valid()){
            tlib::printf("mkfs: open error: %s\n", std::error_message(fd.error()));
            return 1;
        }

        // Get the size of the device

        uint64_t size = 0;
        auto code = tlib::ioctl(*fd, tlib::ioctl_request::GET_BLK_SIZE, &size);

        if(code){
            tlib::printf("mkfs: ioctl error: %s\n", std::error_message(code));
            return 1;
        }

        // Start computing and writing the FAT32 values

        tlib::printf("mkfs: Creating Fat32 filesystem on %s\n", device_str);
        tlib::printf("mkfs: Device size: %m\n", size);

        uint64_t sector_size = 512;
        uint64_t sectors_per_cluster = 8;
        uint64_t sectors = size / sector_size;

        uint64_t available_sectors = sectors - sectors_per_cluster;
        uint64_t available_clusters = available_sectors / sectors_per_cluster;

        // Compute the size of the FAT
        uint64_t fat_size_bytes = available_clusters * sizeof(uint32_t);
        uint64_t fat_size_sectors = std::ceil_divide(fat_size_bytes, sector_size);
        uint64_t fat_size_clusters = std::ceil_divide(fat_size_sectors, sectors_per_cluster);
        fat_size_sectors = fat_size_clusters * sectors_per_cluster;

        tlib::printf("mkfs: Device sectors : %u\n", sectors);
        tlib::printf("mkfs: Available sectors : %u\n", available_sectors);
        tlib::printf("mkfs: FAT sectors : %u\n", fat_size_sectors);

        uint64_t free_clusters = available_clusters - fat_size_clusters - 1;
        uint64_t free_sectors = free_clusters * sectors_per_cluster;

        tlib::printf("mkfs: Free sectors : %u\n", free_sectors);
        tlib::printf("mkfs: Free clusters : %u\n", free_clusters);

        auto fat_bs = std::make_unique<fat32::fat_bs_t>();

        fat_bs->bytes_per_sector = sector_size;
        fat_bs->sectors_per_cluster = sectors_per_cluster;
        fat_bs->reserved_sectors = sectors_per_cluster;
        fat_bs->number_of_fat = 1;
        fat_bs->root_directories_entries = 0;
        fat_bs->total_sectors = 0;
        fat_bs->total_sectors_long = sectors;
        fat_bs->sectors_per_fat = 0;
        fat_bs->sectors_per_fat_long = fat_size_sectors;
        fat_bs->root_directory_cluster_start = 2;
        fat_bs->fs_information_sector = 1;
        std::copy_n("FAT32", 5, &fat_bs->file_system_type[0]);
        fat_bs->signature = 0xAA55;

        // Write the FAT BS
        auto status = tlib::write(*fd, reinterpret_cast<const char*>(fat_bs.get()), sector_size, 0);

        if(!status.valid()){
            tlib::printf("mkfs: write error: %s\n", std::error_message(status.error()));
            return 1;
        }

        auto fat_is = std::make_unique<fat32::fat_is_t>();

        fat_is->allocated_clusters = 1;
        fat_is->free_clusters = free_clusters;
        fat_is->signature_start = 0x52526141;
        fat_is->signature_middle = 0x72724161;
        fat_is->signature_end = 0x000055AA;

        // Write the FAT IS
        status = tlib::write(*fd, reinterpret_cast<const char*>(fat_is.get()), sector_size, sector_size);

        if(!status.valid()){
            tlib::printf("mkfs: write error: %s\n", std::error_message(status.error()));
            return 1;
        }

        // Clear the FAT

        auto fat_begin = fat_bs->reserved_sectors;
        status = tlib::clear(*fd, fat_begin * sector_size, fat_size_sectors * sector_size);

        if(!status.valid()){
            tlib::printf("mkfs: clear error: %s\n", std::error_message(status.error()));
            return 1;
        }

        // Write end of chain for cluster 2 (root)
        uint32_t end_of_cluster_chain = 0x0FFFFFF8;
        status = tlib::write(*fd, reinterpret_cast<char*>(&end_of_cluster_chain), 4, fat_begin * sector_size + 2 * 4);

        if(!status.valid()){
            tlib::printf("mkfs: write error: %s\n", std::error_message(status.error()));
            return 1;
        }

        // Write the root cluster

        std::unique_heap_array<fat32::cluster_entry> root_cluster_entries(16 * fat_bs->sectors_per_cluster);

        //Mark everything as unused
        for(size_t j = 0; j < root_cluster_entries.size() - 1; ++j){
            root_cluster_entries[j].name[0] = 0xE5;
        }

        //End of directory
        root_cluster_entries[root_cluster_entries.size() - 1].name[0] = 0x0;

        //Write the directory entries to the disk
        auto root_sector = fat_begin + (fat_bs->number_of_fat * fat_bs->sectors_per_fat_long);
        if(!tlib::write(*fd, reinterpret_cast<char*>(root_cluster_entries.get()), sector_size * fat_bs->sectors_per_cluster, root_sector * sector_size)){
            return std::ERROR_FAILED;
        }

        return 0;
    }

    tlib::printf("mkfs: Unsupported filesystem %s\n", fs_str);

    return 1;
}
