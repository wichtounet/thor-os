//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <types.hpp>
#include <unique_ptr.hpp>
#include <algorithms.hpp>

#include <tlib/errors.hpp>

#include "fs/fat32.hpp"

#include "drivers/rtc.hpp"

#include "console.hpp"
#include "logging.hpp"

#ifdef THOR_CONFIG_FAT32_VERBOSE
#define verbose_logf(...) logging::logf(__VA_ARGS__)
#else
#define verbose_logf(...)
#endif

namespace {

constexpr const uint32_t CLUSTER_FREE = 0x0;
constexpr const uint32_t CLUSTER_RESERVED= 0x1;
constexpr const uint32_t CLUSTER_CORRUPTED = 0x0FFFFFF7;
constexpr const uint32_t CLUSTER_END = 0x0FFFFFF8;

//Indicates if the entry is unused, indicating a file deletion or move
inline bool entry_unused(const fat32::cluster_entry& entry){
    return entry.name[0] == 0xE5;
}

//Indicates if the entry marks the end of the directory
inline bool end_of_directory(const fat32::cluster_entry& entry){
    return entry.name[0] == 0x0;
}

//Indicates if the entry denotes a long file name entry
inline bool is_long_name(const fat32::cluster_entry& entry){
    return entry.attrib == 0x0F;
}

//Return the number of entries necessary to hold the name
//Always computed to store a long file name entry before the information entry
size_t number_of_entries(const std::string_view& name){
    return (name.size() - 1) / 13 + 2;
}

//Init an entry
template<bool Long>
fat32::cluster_entry* init_entry(fat32::cluster_entry* entry_ptr, std::string_view name, uint32_t cluster){
    //If necessary create all the long filename entries
    if(Long){
        auto len = name.size();

        //Compute the checksum of 8.3 Entry
        char sum = 0;
        for(unsigned int c = 0; c < 11; ++c){
            char v = c < len ? name[c] : ' ';
            sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + v;
        }

        size_t sequences = (len - 1) / 13 + 1;

        size_t sequence = sequences;

        do {
            --sequence;

            auto& l_entry = reinterpret_cast<fat32::long_entry&>(*entry_ptr);

            if(sequence == sequences - 1){
                l_entry.sequence_number = (sequence + 1) | 0x40;
            } else {
                l_entry.sequence_number = sequence + 1;
            }

            l_entry.attrib = 0x0F;
            l_entry.reserved = 0x0;
            l_entry.starting_cluster = 0x0;
            l_entry.alias_checksum = sum;

            size_t i = sequence * 13;

            bool null = false;

            for(size_t j = 0; j < 5; ++j){
                if(i < len){
                    l_entry.name_first[j] = static_cast<uint16_t>(name[i++]);
                } else {
                    l_entry.name_first[j] = !null ? 0x0 : 0xFF;
                    null = true;
                }
            }

            for(size_t j = 0; j < 6; ++j){
                if(i < len){
                    l_entry.name_second[j] = static_cast<uint16_t>(name[i++]);
                } else {
                    l_entry.name_second[j] = !null ? 0x0 : 0xFF;
                    null = true;
                }
            }

            for(size_t j = 0; j < 2; ++j){
                if(i < len){
                    l_entry.name_third[j] = static_cast<uint16_t>(name[i++]);
                } else {
                    l_entry.name_third[j] = !null ? 0x0 : 0xFF;
                    null = true;
                }
            }

            ++entry_ptr;
        } while (sequence != 0);
    }

    auto& entry = *entry_ptr;

    //Copy the name into the entry
    size_t i = 0;
    for(; i < 11 && i < name.size(); ++i){
        entry.name[i] = name[i];
    }

    for(; i < 11; ++i){
        entry.name[i] = ' ';
    }

    auto datetime = rtc::all_data();

    //Set the date and time of the entries
    entry.creation_time_seconds = 0;

    entry.creation_time = datetime.seconds | datetime.minutes << 5 | datetime.hour << 11;
    entry.modification_time = datetime.seconds | datetime.minutes << 5 | datetime.hour << 11;

    entry.creation_date = datetime.day | datetime.month << 5 | (datetime.year - 1980) << 9;
    entry.modification_date = datetime.day | datetime.month << 5 | (datetime.year - 1980) << 9;
    entry.accessed_date = datetime.day | datetime.month << 5 | (datetime.year - 1980) << 9;

    //Clear reserved bits
    entry.reserved = 0;

    //By default file size is zero
    entry.file_size = 0;

    entry.cluster_low = static_cast<uint16_t>(cluster);
    entry.cluster_high = static_cast<uint16_t>(cluster >> 16);

    return entry_ptr;
}

//Init a directory entry
template<bool Long>
void init_directory_entry(fat32::cluster_entry* entry_ptr, std::string_view name, uint32_t cluster){
    //Init the base entry parameters
    entry_ptr = init_entry<Long>(entry_ptr, name, cluster);

    //Mark it as a directory
    entry_ptr->attrib = 1 << 4;
}

//Init a file entry
template<bool Long>
void init_file_entry(fat32::cluster_entry* entry_ptr, std::string_view name, uint32_t cluster){
    //Init the base entry parameters
    entry_ptr = init_entry<Long>(entry_ptr, name, cluster);

    //Mark it as a  file
    entry_ptr->attrib = 0;
}

} //end of anonymous namespace

fat32::fat32_file_system::fat32_file_system(path mount_point, path device) : mount_point(mount_point), device(device) {
    //Nothing else to init
}

fat32::fat32_file_system::~fat32_file_system(){
    delete fat_bs;
    delete fat_is;
}

void fat32::fat32_file_system::init(){
    auto fat_bs_tmp = std::make_unique<fat_bs_t>();

    if(read_sectors(0, 1, fat_bs_tmp.get())){
        fat_bs = fat_bs_tmp.unlock();

        //TODO fat_bs->signature should be 0xAA55
        //TODO fat_bs->file_system_type should be FAT32
    } else {
        fat_bs = nullptr;
    }

    auto fs_information_sector =  static_cast<uint64_t>(fat_bs->fs_information_sector);

    auto fat_is_tmp = std::make_unique<fat_is_t>();

    if(read_sectors(fs_information_sector, 1, fat_is_tmp.get())){
        fat_is = fat_is_tmp.unlock();

        //TODO fat_is->signature_start should be 0x52 0x52 0x61 0x41
        //TODO fat_is->signature_middle should be 0x72 0x72 0x41 0x61
        //TODO fat_is->signature_end should be 0x00 0x00 0x55 0xAA
    } else {
        fat_is = nullptr;
    }

    logging::logf(logging::log_level::TRACE, "fat32: Number of fat:%u\n", uint64_t(fat_bs->number_of_fat));
}

size_t fat32::fat32_file_system::get_file(const path& file_path, vfs::file& file){
    auto all_files = files(file_path, 1);
    for(auto& f : all_files){
        if(f.file_name == file_path.base_name()){
            file = f;
            return 0;
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t fat32::fat32_file_system::read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read){
    verbose_logf(logging::log_level::TRACE, "fat32: Start read (buffer=%p,count=%d,offset=%d)\n", buffer, count, offset);

    vfs::file file;
    auto result = get_file(file_path, file);
    if(result > 0){
        verbose_logf(logging::log_level::TRACE, "fat32: invalid file\n");
        return result;
    }

    uint32_t cluster_number = file.location;
    size_t file_size = file.size;

    //Check the offset parameter
    if(offset > file_size){
        verbose_logf(logging::log_level::TRACE, "fat32: invalid offset\n");
        return std::ERROR_INVALID_OFFSET;
    }

    //No need to read the cluster if there are no content
    if(file_size == 0 || count == 0){
        verbose_logf(logging::log_level::TRACE, "fat32: nothing to read\n");
        read = 0;
        return 0;
    }

    size_t first = offset;
    size_t last = std::min(offset + count, file_size);

    size_t read_bytes = 0;
    size_t position = 0;
    size_t cluster = 0;

    size_t cluster_size = 512 * fat_bs->sectors_per_cluster;

    // Allocate a buffer big enough to read one cluster (possibly several sectors)
    std::unique_heap_array<char> cluster_buffer(cluster_size);

    while(read_bytes < last){
        auto cluster_last = (cluster + 1) * cluster_size;

        if(first < cluster_last){
            verbose_logf(logging::log_level::TRACE, "fat32: read_sectors\n");

            if(read_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, cluster_buffer.get())){
                size_t i = 0;

                if(position == 0){
                    i = first % cluster_size;
                }

                for(; i < cluster_size && read_bytes < last; ++i, ++read_bytes){
                    buffer[position++] = cluster_buffer[i];
                }
            } else {
                verbose_logf(logging::log_level::TRACE, "fat32: read failed\n");

                return std::ERROR_FAILED;
            }
        } else {
            read_bytes += cluster_size;
        }

        ++cluster;

        //If the file is not read completely, get the next cluster
        if(read_bytes < last){
            cluster_number = next_cluster(cluster_number);

            //It may be possible that either the file size or the FAT entry is wrong
            if(!cluster_number){
                break;
            }

            //The block is corrupted
            if(cluster_number == 0x0FFFFFF7){
                break;
            }
        }
    }

    read = last - first;

    verbose_logf(logging::log_level::TRACE, "fat32: finished read\n");

    return 0;
}

size_t fat32::fat32_file_system::read(const path& /*file_path*/, char* /*buffer*/, size_t /*count*/, size_t /*offset*/, size_t& /*read*/, size_t /*ms*/){
    return std::ERROR_UNSUPPORTED;
}

size_t fat32::fat32_file_system::write(const path& file_path, const char* buffer, size_t count, size_t offset, size_t& written){
    vfs::file file;
    auto result = get_file(file_path, file);
    if(result > 0){
        return result;
    }

    uint32_t cluster_number = file.location;
    size_t file_size = file.size;

    //Check the offset parameter
    if(offset + count > file_size){
        return std::ERROR_INVALID_OFFSET;
    }

    //No need to write the cluster if there are no content to write
    if(count == 0){
        written = 0;
        return 0;
    }

    //TODO Change the date of the file

    size_t first = offset;
    size_t last = offset + count;

    size_t read_bytes = 0;
    size_t position = 0;
    size_t cluster = 0;

    size_t cluster_size = 512 * fat_bs->sectors_per_cluster;

    // Allocate a buffer big enough to read one cluster (possibly several sectors)
    std::unique_heap_array<char> cluster_buffer(cluster_size);

    while(read_bytes < last){
        auto cluster_last = (cluster + 1) * cluster_size;

        if(first < cluster_last){

            if(read_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, cluster_buffer.get())){
                size_t i = 0;

                if(position == 0){
                    i = first % cluster_size;
                }

                for(; i < cluster_size && read_bytes < last; ++i, ++read_bytes){
                    cluster_buffer[i] = buffer[position++];
                }

                if(!write_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, cluster_buffer.get())){
                    return std::ERROR_FAILED;
                }
            } else {
                return std::ERROR_FAILED;
            }

        } else {
            read_bytes += cluster_size;
        }

        ++cluster;

        //If the file is not read completely, get the next cluster
        if(read_bytes < last){
            cluster_number = next_cluster(cluster_number);

            //It may be possible that either the file size or the FAT entry is wrong
            if(!cluster_number){
                break;
            }

            //The block is corrupted
            if(cluster_number == 0x0FFFFFF7){
                break;
            }
        }
    }

    written = last - first;

    return 0;
}

size_t fat32::fat32_file_system::clear(const path& file_path, size_t count, size_t offset, size_t& written){
    vfs::file file;
    auto result = get_file(file_path, file);
    if(result > 0){
        return result;
    }

    uint32_t cluster_number = file.location;
    size_t file_size = file.size;

    //Check the offset parameter
    if(offset + count > file_size){
        return std::ERROR_INVALID_OFFSET;
    }

    //No need to write the cluster if there are no content to write
    if(count == 0){
        written = 0;
        return 0;
    }

    //TODO Change the date of the file

    size_t first = offset;
    size_t last = offset + count;

    size_t read_bytes = 0;
    size_t cluster = 0;
    bool first_position = true;

    size_t cluster_size = 512 * fat_bs->sectors_per_cluster;

    // Allocate a buffer big enough to read one cluster (possibly several sectors)
    std::unique_heap_array<char> cluster_buffer(cluster_size);

    while(read_bytes < last){
        auto cluster_last = (cluster + 1) * cluster_size;

        if(first < cluster_last){
            if(read_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, cluster_buffer.get())){
                size_t i = 0;

                if(first_position){
                    i = first % cluster_size;
                    first_position = false;
                }

                for(; i < cluster_size && read_bytes < last; ++i, ++read_bytes){
                    cluster_buffer[i] = 0;
                }

                if(!write_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, cluster_buffer.get())){
                    return std::ERROR_FAILED;
                }
            } else {
                return std::ERROR_FAILED;
            }

        } else {
            read_bytes += cluster_size;
        }

        ++cluster;

        //If the file is not read completely, get the next cluster
        if(read_bytes < last){
            cluster_number = next_cluster(cluster_number);

            //It may be possible that either the file size or the FAT entry is wrong
            if(!cluster_number){
                break;
            }

            //The block is corrupted
            if(cluster_number == 0x0FFFFFF7){
                break;
            }
        }
    }

    written = last - first;

    return 0;
}

size_t fat32::fat32_file_system::truncate(const path& file_path, size_t file_size){
    vfs::file file;
    auto result = get_file(file_path, file);
    if(result > 0){
        return result;
    }

    if(file.size == file_size){
        return 0;
    }

    //Find the cluster number of the parent directory
    auto parent_cluster_number_search = find_cluster_number(file_path, 1);
    if(!parent_cluster_number_search.first){
        return std::ERROR_NOT_EXISTS;
    }

    //TODO Change the date of the file

    //If we need to increase the size
    if(file.size < file_size){
        auto cluster_size = 512 * fat_bs->sectors_per_cluster;
        auto clusters = file_size % cluster_size == 0 ? file_size / cluster_size : (file_size / cluster_size) + 1;

        size_t capacity = 0;
        auto last_cluster = file.location;

        if(last_cluster >= 2){
            ++capacity;

            while(true){
                auto next = next_cluster(last_cluster);

                if(!next || next == CLUSTER_CORRUPTED){
                    break;
                }

                last_cluster = next;
                ++capacity;
            }
        }

        if(capacity < clusters){
            if(capacity == 0){
                auto cluster = find_free_cluster();
                if(!cluster){
                    return std::ERROR_DISK_FULL;
                }

                --fat_is->free_clusters;

                last_cluster = cluster;

                ++capacity;

                change_directory_entry(parent_cluster_number_search.second, file.position,
                    [cluster](cluster_entry& entry){
                    entry.cluster_low = cluster;
                    entry.cluster_high = cluster >> 16;
                    });
            }

            //Extend the clusters if necessary
            for(auto i = capacity; i < clusters; ++i){
                auto cluster = find_free_cluster();
                if(!cluster){
                    return std::ERROR_DISK_FULL;
                }

                --fat_is->free_clusters;

                //Update the cluster chain
                if(!write_fat_value(last_cluster, cluster)){
                    return std::ERROR_FAILED;
                }

                last_cluster = cluster;
            }

            //Update the cluster chain
            if(!write_fat_value(last_cluster, CLUSTER_END)){
                return std::ERROR_FAILED;
            }

            if(!write_is()){
                return std::ERROR_FAILED;
            }
        }
    }

    //Set the new file size in the directory entry
    change_directory_entry(parent_cluster_number_search.second, file.position,
        [file_size](cluster_entry& entry){ entry.file_size = file_size; });

    return 0;
}

size_t fat32::fat32_file_system::ls(const path& file_path, std::vector<vfs::file>& contents){
    //TODO Better handling of error inside files()
    contents = files(file_path);

    return 0;
}

size_t fat32::fat32_file_system::touch(const path& file_path){
    //Find the cluster number of the parent directory
    auto cluster_number = find_cluster_number(file_path, 1);
    if(!cluster_number.first){
        return std::ERROR_NOT_EXISTS;
    }

    auto parent_cluster_number = cluster_number.second;

    std::unique_heap_array<cluster_entry> directory_cluster(16 * fat_bs->sectors_per_cluster);
    if(!read_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
        return std::ERROR_FAILED;
    }

    auto file = file_path.base_name();

    auto entries = number_of_entries(file);
    auto new_directory_entry = find_free_entry(directory_cluster, entries, parent_cluster_number);

    init_file_entry<true>(new_directory_entry, file, 0);

    //Write back the parent directory cluster
    if(!write_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
        return std::ERROR_FAILED;
    }

    return 0;
}

size_t fat32::fat32_file_system::mkdir(const path& file_path){
    //Find the cluster number of the parent directory
    auto cluster_number = find_cluster_number(file_path, 1);
    if(!cluster_number.first){
        return std::ERROR_NOT_EXISTS;
    }

    const auto parent_cluster = cluster_number.second;

    //Find a free cluster to hold the directory entries
    const auto cluster = find_free_cluster();
    if(cluster == 0){
        return std::ERROR_DISK_FULL;
    }

    verbose_logf(logging::log_level::TRACE, "fat32: mkdir: free_cluster:%u\n", size_t(cluster));
    verbose_logf(logging::log_level::TRACE, "fat32: mkdir: parent_cluster:%u\n", size_t(parent_cluster));

    std::unique_heap_array<cluster_entry> directory_cluster(16 * fat_bs->sectors_per_cluster);
    if(!read_sectors(cluster_lba(parent_cluster), fat_bs->sectors_per_cluster, directory_cluster.get())){
        return std::ERROR_FAILED;
    }

    auto directory = file_path.base_name();

    auto parent_cluster_number = parent_cluster; // This may change if full
    auto entries = number_of_entries(directory);
    auto new_directory_entry = find_free_entry(directory_cluster, entries, parent_cluster_number);

    init_directory_entry<true>(new_directory_entry, directory, cluster);

    //Write back the parent directory cluster
    if(!write_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
        return std::ERROR_FAILED;
    }

    //This cluster is the end of the chain
    if(!write_fat_value(cluster, CLUSTER_END)){
        return std::ERROR_FAILED;
    }

    //One cluster is now used for the directory entries
    --fat_is->free_clusters;
    if(!write_is()){
        return std::ERROR_FAILED;
    }

    //Update the directory entries
    std::unique_heap_array<cluster_entry> new_directory_cluster(16 * fat_bs->sectors_per_cluster);

    auto dot_entry = &new_directory_cluster[0];
    init_directory_entry<false>(dot_entry, ".", cluster);

    auto dot_dot_entry = &new_directory_cluster[1];
    init_directory_entry<false>(dot_dot_entry, "..", parent_cluster == fat_bs->root_directory_cluster_start ? 0 : parent_cluster);

    //Mark everything as unused
    for(size_t j = 2; j < new_directory_cluster.size() - 1; ++j){
        new_directory_cluster[j].name[0] = 0xE5;
    }

    //End of directory
    new_directory_cluster[new_directory_cluster.size() - 1].name[0] = 0x0;

    //Write the directory entries to the disk
    if(!write_sectors(cluster_lba(cluster), fat_bs->sectors_per_cluster, new_directory_cluster.get())){
        return std::ERROR_FAILED;
    }

    verbose_logf(logging::log_level::TRACE, "fat32: mkdir: special entry . -> %u\n", size_t(new_directory_cluster[0].cluster_low));
    verbose_logf(logging::log_level::TRACE, "fat32: mkdir: special entry . -> %u\n", size_t(new_directory_cluster[1].cluster_low));

    return 0;
}

size_t fat32::fat32_file_system::rm(const path& file_path){
    vfs::file file;
    auto result = get_file(file_path, file);
    if(result > 0){
        return result;
    }

    uint32_t cluster_number = file.location;
    bool is_file = !file.directory;
    size_t position = file.position;

    //Find the cluster number of the parent directory
    auto cluster_number_search = find_cluster_number(file_path, 1);
    if(!cluster_number_search.first){
        return false;
    }

    auto parent_cluster_number = cluster_number_search.second;

    if(is_file){
        return rm_file(parent_cluster_number, position, cluster_number);
    } else {
        return rm_dir(parent_cluster_number, position, cluster_number);
    }
}

size_t fat32::fat32_file_system::statfs(vfs::statfs_info& file){
    file.total_size = fat_bs->total_sectors_long * 512;
    file.free_size = fat_is->free_clusters * fat_bs->sectors_per_cluster * 512;

    return 0;
}

/* Private methods implementation */

size_t fat32::fat32_file_system::rm_file(uint32_t parent_cluster_number, size_t position, uint32_t cluster_number){
    std::unique_heap_array<cluster_entry> directory_cluster(16 * fat_bs->sectors_per_cluster);
    if(!read_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
        return std::ERROR_FAILED;
    }

    //1. Mark the entries in directory as unused

    bool found = false;
    size_t cluster_position = 0;
    while(!found){
        bool end_reached = false;

        //Verify if is the correct cluster
        if(position >= cluster_position * directory_cluster.size() && position < (cluster_position + 1) * directory_cluster.size()){
            found = true;

            auto j = position % directory_cluster.size();
            directory_cluster[j].name[0] = 0xE5;

            while(is_long_name(directory_cluster[--j])){
                directory_cluster[j].name[0] = 0xE5;

                if(j == 0){
                    break;
                }
            }

            if(!write_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
                return std::ERROR_FAILED;
            }
        }

        //Jump to next cluser
        if(!found && !end_reached){
            parent_cluster_number = next_cluster(parent_cluster_number);

            if(!parent_cluster_number){
                break;
            }

            //The block is corrupted
            if(parent_cluster_number == CLUSTER_CORRUPTED){
                break;
            }

            if(!read_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
                break;
            }
        }
    }

    if(!found){
        return std::ERROR_NOT_EXISTS;
    }

    //2. Release all the clusters of the chain
    if(cluster_number >= 2){
        while(true){
            auto next = next_cluster(cluster_number);

            //Mark this cluster as unused
            if(!write_fat_value(cluster_number, CLUSTER_FREE)){
                return std::ERROR_FAILED;
            }

            ++fat_is->free_clusters;

            if(!next || next == CLUSTER_CORRUPTED){
                break;
            }

            cluster_number = next;
        }
    }

    if(!write_is()){
        return std::ERROR_FAILED;
    }

    return 0;
}

size_t fat32::fat32_file_system::rm_dir(uint32_t parent_cluster_number, size_t position, uint32_t cluster_number){
    //1. Every sub entry of the directory must be removed

    for(auto& file : files(cluster_number)){
        if(file.file_name == "." || file.file_name == ".."){
            continue;
        }

        if(file.directory){
            auto result = rm_dir(cluster_number, file.position, file.location);
            if(result > 0){
                return result;
            }
        } else {
            auto result = rm_file(cluster_number, file.position, file.location);
            if(result > 0){
                return result;
            }
        }
    }

    //Once the sub entries have been removed, a directory can be removed
    //as a file
    return rm_file(parent_cluster_number, position, cluster_number);
}

size_t fat32::fat32_file_system::change_directory_entry(uint32_t parent_cluster_number, size_t position, const std::function<void(cluster_entry&)>& functor){
    std::unique_heap_array<cluster_entry> directory_cluster(16 * fat_bs->sectors_per_cluster);
    if(!read_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
        return std::ERROR_FAILED;
    }

    //1. Mark the entries in directory as unused

    bool found = false;
    size_t cluster_position = 0;
    while(!found){
        bool end_reached = false;

        //Verify if is the correct cluster
        if(position >= cluster_position * directory_cluster.size() && position < (cluster_position + 1) * directory_cluster.size()){
            found = true;

            auto j = position % directory_cluster.size();

            functor(directory_cluster[j]);

            if(!write_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
                return std::ERROR_FAILED;
            }
        }

        //Jump to next cluser
        if(!found && !end_reached){
            parent_cluster_number = next_cluster(parent_cluster_number);

            if(!parent_cluster_number){
                break;
            }

            //The block is corrupted
            if(parent_cluster_number == CLUSTER_CORRUPTED){
                break;
            }

            if(!read_sectors(cluster_lba(parent_cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
                break;
            }
        }
    }

    if(!found){
        return std::ERROR_NOT_EXISTS;
    }

    return 0;
}

//Finds "entries" consecutive free entries in the given directory cluster
fat32::cluster_entry* fat32::fat32_file_system::find_free_entry(std::unique_heap_array<cluster_entry>& directory_cluster, size_t entries, uint32_t& cluster_number){
    while(true){
        size_t end = 0;
        bool end_found = false;

        //1. Search the first end of directory marker
        for(size_t i = 0; i < directory_cluster.size(); ++i){
            auto& entry = directory_cluster[i];

            if(end_of_directory(entry)){
                end = i;
                end_found = true;
                break;
            }
        }

        if(end_found){
            //2. Find a big-enough sequence of free entries
            size_t sequence_size = 0;
            size_t sequence_start = 0;
            size_t sequence_end = 0;

            for(size_t i = 0; i < directory_cluster.size(); ++i){
                auto& entry = directory_cluster[i];

                if(end_of_directory(entry) || entry_unused(entry)){
                    ++sequence_size;

                    if(sequence_size == entries){
                        sequence_start = i - (sequence_size - 1);
                        sequence_end = i;
                        break;
                    }
                } else {
                    sequence_size = 0;
                }
            }

            if(sequence_size == entries){
                bool ok = true;

                //If the free sequence position is after end or if end  is inside the free
                //sequence
                if(end <= sequence_end){
                    int64_t new_end = -1;
                    for(size_t i = directory_cluster.size() - 1; i > 0; --i){
                        if(i <= sequence_end){
                            break;
                        }

                        auto& entry = directory_cluster[i];
                        if(end_of_directory(entry) || entry_unused(entry)){
                            new_end = i;
                        } else {
                            break;
                        }
                    }

                    //If there is not enough room to hold the entries and
                    //the end of directory marker, try with the next cluster
                    if(new_end < 0){
                        ok = false;
                    } else {
                        //Mark the old end as unused
                        directory_cluster[end].name[0] = 0xE5;

                        //Mark the new end as the end of the directory
                        directory_cluster[new_end].name[0] = 0x0;
                    }
                }

                if(ok){
                    return &directory_cluster[sequence_start];
                }
            }
        }

        //If there is an end and we have to go to the next cluster,
        //remove the end of directory markers
        if(end_found){
            //Remove all the end of directory marker in the cluster
            for(auto& entry : directory_cluster){
                if(end_of_directory(entry)){
                    entry.name[0] = 0xE5;
                }
            }

            //Write back the cluster
            if(!write_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
                return nullptr;
            }
        }

        //3. Go to the next cluster
        auto next = next_cluster(cluster_number);

        //If there are no next blocks, exit the loop and extend the
        //directory cluster chain
        if(!next){
            break;
        }

        //If the block is corrupted, we do not try to do anything else
        if(next == CLUSTER_CORRUPTED){
            return nullptr;
        }

        //Try again with the cluster in the chain
        cluster_number = next;

        //Read the next sector
        if(!read_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
            return nullptr;
        }
    }

    //At this point, we tried all the possible clusters of the directory,
    //So it is necessary to add a new cluster to the chain
    return extend_directory(directory_cluster, entries, cluster_number);
}

fat32::cluster_entry* fat32::fat32_file_system::extend_directory(std::unique_heap_array<fat32::cluster_entry>& directory_cluster, size_t entries, uint32_t& cluster_number){
    auto cluster = find_free_cluster();
    if(!cluster){
        return nullptr;
    }

    --fat_is->free_clusters;
    if(!write_is()){
        return nullptr;
    }

    //Update the cluster chain
    if(!write_fat_value(cluster_number, cluster)){
        return nullptr;
    }

    if(!write_fat_value(cluster, CLUSTER_END)){
        return nullptr;
    }

    //Remove all the end of directory marker in the previous cluster
    for(auto& entry : directory_cluster){
        if(end_of_directory(entry)){
            entry.name[0] = 0xE5;
        }
    }

    //Write back the previous cluster
    if(!write_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, directory_cluster.get())){
        return nullptr;
    }

    cluster_number = cluster;

    //In the new directory, mark all entries, but the first entries one as
    //end of directory
    for(size_t i = entries; i < directory_cluster.size(); ++i){
        auto& entry = directory_cluster[i];

        entry.name[0] = 0x0;
        entry.attrib = 0x0;
    }

    return &directory_cluster[0];
}

//Return all the files of the given directory (denoted by its cluster number)
std::vector<vfs::file> fat32::fat32_file_system::files(uint32_t cluster_number){
    std::vector<vfs::file> files;

    bool end_reached = false;

    bool long_name = false;
    char long_name_buffer[256];
    size_t i = 0;
    size_t cluster_position = 0;

    std::unique_heap_array<fat32::cluster_entry> cluster(16 * fat_bs->sectors_per_cluster);

    while(!end_reached){
        if(!read_sectors(cluster_lba(cluster_number), fat_bs->sectors_per_cluster, cluster.get())){
            return files;
        }

        size_t position = 0;
        for(auto& entry : cluster){
            ++position;

            if(end_of_directory(entry)){
                end_reached = true;
                break;
            }

            if(entry_unused(entry)){
                continue;
            }

            if(is_long_name(entry)){
                auto& l_entry = reinterpret_cast<fat32::long_entry&>(entry);
                long_name = true;

                size_t l = ((l_entry.sequence_number & ~0x40) - 1) * 13;

                bool end = false;
                for(size_t j = 0; !end && j < 5; ++j){
                    if(l_entry.name_first[j] == 0x0 || l_entry.name_first[j] == 0xFF){
                        end = true;
                    } else {
                        char c = static_cast<char>(l_entry.name_first[j]);
                        long_name_buffer[l++] = c;
                    }
                }

                for(size_t j = 0; !end && j < 6; ++j){
                    if(l_entry.name_second[j] == 0x0 || l_entry.name_second[j] == 0xFF){
                        end = true;
                    } else {
                        char c = static_cast<char>(l_entry.name_second[j]);
                        long_name_buffer[l++] = c;
                    }
                }

                for(size_t j = 0; !end && j < 2; ++j){
                    if(l_entry.name_third[j] == 0x0 || l_entry.name_third[j] == 0xFF){
                        end = true;
                    } else {
                        char c = static_cast<char>(l_entry.name_third[j]);
                        long_name_buffer[l++] = c;
                    }
                }

                if(l > i){
                    i = l;
                }

                continue;
            }

            vfs::file file;

            if(long_name){
                for(size_t s = 0; s < i; ++s){
                    file.file_name += long_name_buffer[s];
                }

                long_name = false;
                i = 0;
            } else {
                //It is a normal file name
                //Copy the name until the first space

                for(size_t s = 0; s < 11; ++s){
                    if(entry.name[s] == ' '){
                        break;
                    }

                    file.file_name += entry.name[s];
                }
            }

            file.hidden = entry.attrib & 0x1;
            file.system = entry.attrib & 0x2;
            file.directory = entry.attrib & 0x10;

            file.created.day = entry.creation_date & 0x1F;
            file.created.month = (entry.creation_date >> 5) & 0xF;
            file.created.year = (entry.creation_date >> 9) + 1980;

            file.created.seconds = entry.creation_time & 0x1F;
            file.created.minutes = (entry.creation_time >> 5) & 0x3F;
            file.created.hour = entry.creation_time >> 11;

            file.modified.day = entry.modification_date & 0x1F;
            file.modified.month = (entry.modification_date >> 5) & 0xF;
            file.modified.year = (entry.modification_date >> 9) + 1980;

            file.modified.seconds = entry.modification_time & 0x1F;
            file.modified.minutes = (entry.modification_time >> 5) & 0x3F;
            file.modified.hour = entry.modification_time >> 11;

            file.accessed.day = entry.accessed_date & 0x1F;
            file.accessed.month = (entry.accessed_date >> 5) & 0xF;
            file.accessed.year = (entry.accessed_date >> 9) + 1980;

            if(file.directory){
                //TODO Should read the cluster chain to get the number of
                //clusters
                file.size = fat_bs->sectors_per_cluster * 512;
            } else {
                file.size = entry.file_size;
            }

            file.location = (uint32_t(entry.cluster_high) << 16) + uint32_t(entry.cluster_low);
            file.position = cluster_position * cluster.size() + (position - 1);

            if(file.location == 0){
                file.location = fat_bs->root_directory_cluster_start;
            }

            files.push_back(file);
        }

        if(!end_reached){
            cluster_number = next_cluster(cluster_number);

            //If there are no more cluster, return false
            if(!cluster_number){
                return files;
            }

            //The block is corrupted
            if(cluster_number == CLUSTER_CORRUPTED){
                return files;
            }
        }

        ++cluster_position;
    }

    return files;
}

//TODO use expected here
//Find the cluster for the given path
std::pair<bool, uint32_t> fat32::fat32_file_system::find_cluster_number(const path& file_path, size_t last){
    auto cluster_number = fat_bs->root_directory_cluster_start;

    if(file_path.size() - last == 1){
        return std::make_pair(true, cluster_number);
    }

    for(size_t i = 1; i < file_path.size() - last; ++i){
        auto p = file_path[i];

        bool found = false;

        auto entries = files(cluster_number);

        for(auto& file : entries){
            if(i == file_path.size() - 1 - last || file.directory){
                if(file.file_name == p){
                    cluster_number = file.location;

                    //If it is the last part of the path, just return the
                    //number
                    if(i == file_path.size() - 1 - last){
                        return std::make_pair(true, cluster_number);
                    }

                    //Otherwise, continue with the next level of the path

                    found = true;

                    break;
                }
            }
        }

        if(!found){
            return std::make_pair(false, 0);
        }
    }

    return std::make_pair(false, 0);
}

//Return all the files in the directory denoted by its path
//No test is make to verify that the path denotes a directory
std::vector<vfs::file> fat32::fat32_file_system::files(const path& file_path, size_t last){
    auto cluster_number_search = find_cluster_number(file_path, last);
    if(!cluster_number_search.first){
        return {};
    }

    return files(cluster_number_search.second);
}
//Write information sector to the disk
bool fat32::fat32_file_system::write_is(){
    auto fs_information_sector = static_cast<uint64_t>(fat_bs->fs_information_sector);

    return write_sectors(fs_information_sector, 1, fat_is);
}

//Return the absolute sector where the cluster resides
uint64_t fat32::fat32_file_system::cluster_lba(uint64_t cluster){
    uint64_t fat_begin = fat_bs->reserved_sectors;
    uint64_t cluster_begin = fat_begin + (fat_bs->number_of_fat * fat_bs->sectors_per_fat_long);

    return cluster_begin + (cluster - 2) * fat_bs->sectors_per_cluster;
}

//Return the value of the fat for the given cluster
//Return 0 if an error occurs
uint32_t fat32::fat32_file_system::read_fat_value(uint32_t cluster){
    uint64_t fat_begin = fat_bs->reserved_sectors;
    uint64_t fat_sector = fat_begin + (cluster * sizeof(uint32_t)) / 512;

    std::unique_heap_array<uint32_t> fat_table(512 / sizeof(uint32_t));
    if(read_sectors(fat_sector, 1, fat_table.get())){
        uint64_t entry_offset = cluster % (512 / sizeof(uint32_t));
        auto v = fat_table[entry_offset] & 0x0FFFFFFF;
        return v;
    } else {
        return 0;
    }
}

//Write a value to the FAT for the given cluster
bool fat32::fat32_file_system::write_fat_value(uint32_t cluster, uint32_t value){
    const auto fat_sectors = fat_bs->sectors_per_fat_long + fat_bs->sectors_per_fat;

    uint64_t fat_begin = fat_bs->reserved_sectors;

    for(size_t f = 0; f < fat_bs->number_of_fat; ++f){
        uint64_t fat_sector = fat_begin + (cluster * sizeof(uint32_t)) / 512;

        //Read the cluster we need to alter
        std::unique_heap_array<uint32_t> fat_table(512 / sizeof(uint32_t));
        if(!read_sectors(fat_sector, 1, fat_table.get())){
            return false;
        }

        //Set the entry to the given value
        uint64_t entry_offset = cluster % (512 / sizeof(uint32_t));
        fat_table[entry_offset] = value;

        //Write back the cluster
        if(!write_sectors(fat_sector, 1, fat_table.get())){
            return false;
        }

        // Switch to the next FAT
        fat_begin += fat_sectors;
    }

    return true;
}

//Return the next cluster in the chain for the give cluster
//0 indicates that there is no next cluster
uint32_t fat32::fat32_file_system::next_cluster(uint32_t cluster){
    auto fat_value = read_fat_value(cluster);
    if(fat_value >= CLUSTER_END){
        return 0;
    }

    return fat_value;
}

//Find a free cluster in the disk
//0 indicates failure or disk full
uint32_t fat32::fat32_file_system::find_free_cluster(){
    uint64_t fat_begin = fat_bs->reserved_sectors;

    static constexpr const auto entries_per_sector = 512 / sizeof(uint32_t);

    std::unique_heap_array<uint32_t> fat_table(entries_per_sector);

    const auto fat_sectors = fat_bs->sectors_per_fat_long + fat_bs->sectors_per_fat;

    // Iterate over each sector of the FAT
    for(size_t j = 0; j < fat_sectors; ++j){
        uint64_t fat_sector = fat_begin + j;

        // Read one sector of the FAT
        if(!read_sectors(fat_sector, 1, fat_table.get())){
            return 0; //0 is not a valid cluster number, indicates failure
        }

        // Check all cluster
        for(size_t i = 0; i < fat_table.size(); ++i){
            //Cluster 0 and 1 are not valid cluster
            if(j == 0 && i < 2){
                continue;
            }

            auto value = fat_table[i] & 0x0FFFFFFF;
            if(value == CLUSTER_FREE){
                return i + j * entries_per_sector;
            }
        }
    }

    return 0; //0 is not a valid cluster number, indicates failure
}

bool fat32::fat32_file_system::read_sectors(uint64_t start, uint8_t count, void* destination){
    auto result = vfs::direct_read(device, reinterpret_cast<char*>(destination), count * 512, start * 512);
    return result && *result == count * 512;
}

bool fat32::fat32_file_system::write_sectors(uint64_t start, uint8_t count, void* source){
    auto result = vfs::direct_write(device, reinterpret_cast<const char*>(source), count * 512, start * 512);
    return result && *result == count * 512;
}
