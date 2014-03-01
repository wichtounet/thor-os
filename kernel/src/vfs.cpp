//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <errors.hpp>
#include <string.hpp>
#include <algorithms.hpp>

#include "vfs.hpp"
#include "scheduler.hpp"
#include "flags.hpp"

#include <directory_entry.hpp>
#include <mount_point.hpp>

#include "fat32.hpp"

#include "disks.hpp"

#include "console.hpp"

namespace {

struct mounted_fs {
    vfs::partition_type fs_type;
    std::string device;
    std::string mount_point;

    mounted_fs() = default;

    mounted_fs(vfs::partition_type type, const char* dev, const char* mp) :
        fs_type(type), device(dev), mount_point(mp)
    {
        //Nothing to init
    }
};

std::string partition_type_to_string(vfs::partition_type type){
    switch(type){
        case vfs::partition_type::FAT32:
            return "FAT32";
        case vfs::partition_type::UNKNOWN:
            return "Unknown";
        default:
            return "Invalid Type";
    }
}

std::vector<mounted_fs> mount_point_list;

void mount_root(){
    //TODO Get information about the root from a confgiuration file
    mount(vfs::partition_type::FAT32, "/", "TODO");
}

} //end of anonymous namespace

//TODO Remove the direct accesses to fat32

void vfs::init(){
    mount_root();
}

int64_t vfs::mount(partition_type type, const char* mount_point, const char* device){
    //TODO In the future just delegates to the correct file system function
    switch(type){
        case vfs::partition_type::FAT32:
            //TODO Generalize
            disks::mount(disks::disk_by_uuid(0), 0);

            break;
        default:
            return -std::ERROR_INVALID_FILE_SYSTEM;
    }

    mount_point_list.emplace_back(type, device, mount_point);

    return 0;
}

std::vector<std::string> get_path(const char* file_path){
    std::string file(file_path);

    std::vector<std::string> path;

    if(file[0] != '/'){
        for(auto& part : scheduler::get_working_directory()){
            path.push_back(part);
        }
    }

    auto parts = std::split(file, '/');
    for(auto& part : parts){
        path.push_back(part);
    }

    return std::move(path);
}

int64_t vfs::open(const char* file_path, size_t flags){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        return -std::ERROR_NOTHING_MOUNTED;
    }

    std::string file(file_path);

    if(file.empty()){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    auto path = get_path(file_path);

    //Special handling for opening the root
    if(path.empty()){
        return scheduler::register_new_handle(path);
    }

    auto last = path.back();
    path.pop_back();

    if(flags & std::OPEN_CREATE){
        bool success = fat32::touch(*disks::mounted_disk(), *disks::mounted_partition(), path, last);

        if(success){
            return scheduler::register_new_handle(path);
        } else {
            //TODO Use better error directly from touch
            return std::ERROR_FAILED;
        }
    } else {
        //TODO file search should be done entirely by the file system

        auto files = fat32::ls(*disks::mounted_disk(), *disks::mounted_partition(), path);

        for(auto& f : files){
            if(f.file_name == last){
                path.push_back(last);
                return scheduler::register_new_handle(path);
            }
        }

        return -std::ERROR_NOT_EXISTS;
    }
}

void vfs::close(size_t fd){
    if(scheduler::has_handle(fd)){
        scheduler::release_handle(fd);
    }
}

int64_t vfs::mkdir(const char* file_path){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        return -std::ERROR_NOTHING_MOUNTED;
    }

    std::string file(file_path);

    if(file.empty()){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    auto path = get_path(file_path);

    auto last = path.back();
    path.pop_back();

    auto files = fat32::ls(*disks::mounted_disk(), *disks::mounted_partition(), path);

    for(auto& f : files){
        if(f.file_name == last){
            return -std::ERROR_EXISTS;
        }
    }

    bool success = fat32::mkdir(*disks::mounted_disk(), *disks::mounted_partition(), path, last);
    if(!success){
        return -std::ERROR_FAILED;
    } else {
        return 0;
    }
}

int64_t vfs::rm(const char* file_path){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        return -std::ERROR_NOTHING_MOUNTED;
    }

    std::string file(file_path);

    if(file.empty()){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    auto path = get_path(file_path);

    auto last = path.back();
    path.pop_back();

    bool success = fat32::rm(*disks::mounted_disk(), *disks::mounted_partition(), path, last);
    if(!success){
        return -std::ERROR_FAILED;
    } else {
        return 0;
    }
}

int64_t vfs::stat(size_t fd, stat_info& info){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        return -std::ERROR_NOTHING_MOUNTED;
    }

    if(!scheduler::has_handle(fd)){
        return -std::ERROR_INVALID_FILE_DESCRIPTOR;
    }

    if(scheduler::get_handle(fd).empty()){
    }

    auto path = scheduler::get_handle(fd);

    //Special handling for root
    if(path.empty()){
        //TODO Add file system support for stat of the root directory
        info.size = 4096;
        info.flags = STAT_FLAG_DIRECTORY;

        return 0;
    }

    auto last = path.back();
    path.pop_back();

    //TODO file search should be done entirely by the file system

    auto files = fat32::ls(*disks::mounted_disk(), *disks::mounted_partition(), path);

    for(auto& f : files){
        if(f.file_name == last){
            info.size = f.size;
            info.flags = 0;

            if(f.directory){
                info.flags |= STAT_FLAG_DIRECTORY;
            }

            if(f.system){
                info.flags |= STAT_FLAG_SYSTEM;
            }

            if(f.hidden){
                info.flags |= STAT_FLAG_HIDDEN;
            }

            info.created = f.created;
            info.modified = f.modified;
            info.accessed = f.accessed;

            return 0;
        }
    }

    return -std::ERROR_NOT_EXISTS;
}

int64_t vfs::read(size_t fd, char* buffer, size_t max){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        return -std::ERROR_NOTHING_MOUNTED;
    }

    if(!scheduler::has_handle(fd)){
        return -std::ERROR_INVALID_FILE_DESCRIPTOR;
    }

    auto path = scheduler::get_handle(fd);

    if(path.empty()){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    auto last = path.back();
    path.pop_back();

    //TODO file search should be done entirely by the file system

    auto content = fat32::read_file(*disks::mounted_disk(), *disks::mounted_partition(), path, last);

    if(content.empty()){
        return 0;
    }

    size_t i = 0;
    for(; i < content.size() && i < max; ++i){
        buffer[i] = content[i];
    }

    return i;
}

int64_t vfs::mounts(char* buffer, size_t size){
    size_t total_size = 0;

    for(auto& mp : mount_point_list){
        total_size += 4 * sizeof(size_t) + 3 + mp.device.size() + mp.mount_point.size() + partition_type_to_string(mp.fs_type).size();
    }

    if(size < total_size){
        return -std::ERROR_BUFFER_SMALL;
    }

    size_t position = 0;

    for(size_t i = 0; i < mount_point_list.size(); ++i){
        auto& mp = mount_point_list[i];

        auto entry = reinterpret_cast<mount_point*>(buffer + position);

        auto fs_type = partition_type_to_string(mp.fs_type);

        entry->length_mp = mp.mount_point.size();
        entry->length_dev = mp.device.size();
        entry->length_type = fs_type.size();

        if(i + 1 < mount_point_list.size()){
            entry->offset_next = 4 * sizeof(size_t) + 3 + mp.device.size() + mp.mount_point.size() + fs_type.size();
            position += entry->offset_next;
        } else {
            entry->offset_next = 0;
        }

        char* name_buffer = &(entry->name);
        size_t str_pos = 0;

        for(size_t j = 0; j < mp.mount_point.size(); ++j){
            name_buffer[str_pos++] = mp.mount_point[j];
        }
        name_buffer[str_pos++] = '\0';
        for(size_t j = 0; j < mp.device.size(); ++j){
            name_buffer[str_pos++] = mp.device[j];
        }
        name_buffer[str_pos++] = '\0';
        for(size_t j = 0; j < fs_type.size(); ++j){
            name_buffer[str_pos++] = fs_type[j];
        }
        name_buffer[str_pos++] = '\0';
    }

    return total_size;
}

int64_t vfs::entries(size_t fd, char* buffer, size_t size){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        return -std::ERROR_NOTHING_MOUNTED;
    }

    if(!scheduler::has_handle(fd)){
        return -std::ERROR_INVALID_FILE_DESCRIPTOR;
    }

    auto path = scheduler::get_handle(fd);

    //TODO file search should be done entirely by the file system

    auto files = fat32::ls(*disks::mounted_disk(), *disks::mounted_partition(), path);

    size_t total_size = 0;

    for(auto& f : files){
        total_size += sizeof(directory_entry) + f.file_name.size();
    }

    if(size < total_size){
        return -std::ERROR_BUFFER_SMALL;
    }

    size_t position = 0;

    for(size_t i = 0; i < files.size(); ++i){
        auto& file = files[i];

        auto entry = reinterpret_cast<directory_entry*>(buffer + position);

        entry->type = 0; //TODO Fill that
        entry->length = file.file_name.size();

        if(i + 1 < files.size()){
            entry->offset_next = file.file_name.size() + 1 + 3 * 8;
            position += entry->offset_next;
        } else {
            entry->offset_next = 0;
        }

        char* name_buffer = &(entry->name);
        for(size_t j = 0; j < file.file_name.size(); ++j){
            name_buffer[j] = file.file_name[j];
        }
        name_buffer[file.file_name.size()] = '\0';
    }

    return total_size;
}
