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

#include "fat32.hpp"

#include "disks.hpp"

#include "console.hpp"

//TODO Remove the direct accesses to fat32

void vfs::init(){
    //TODO
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
