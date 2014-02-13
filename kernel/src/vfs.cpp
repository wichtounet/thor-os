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

#include "fat32.hpp"

#include "disks.hpp"

#include "console.hpp"

int64_t vfs::open(const char* file_path){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        return -std::ERROR_NOTHING_MOUNTED;
    }

    std::string file(file_path);

    if(file.empty()){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    if(file[0] != '/'){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    auto parts = std::split(file, '/');

    if(parts.empty()){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    auto last = parts.back();
    parts.pop_back();

    //TODO file search should be done entirely by the file system

    auto files = fat32::ls(*disks::mounted_disk(), *disks::mounted_partition(), parts);

    for(auto& f : files){
        if(f.file_name == last){
            return scheduler::register_new_handle(file);
        }
    }

    return -std::ERROR_NOT_EXISTS;
}

void vfs::close(size_t fd){
    if(scheduler::has_handle(fd)){
        scheduler::release_handle(fd);
    }
}

int64_t vfs::stat(size_t fd, stat_info& info){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        return -std::ERROR_NOTHING_MOUNTED;
    }

    if(!scheduler::has_handle(fd)){
        return -std::ERROR_INVALID_FILE_DESCRIPTOR;
    }

    auto& path = scheduler::get_handle(fd);

    auto parts = std::split(path, '/');

    if(parts.empty()){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    auto last = parts.back();
    parts.pop_back();

    //TODO file search should be done entirely by the file system

    auto files = fat32::ls(*disks::mounted_disk(), *disks::mounted_partition(), parts);

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

    auto& path = scheduler::get_handle(fd);

    auto parts = std::split(path, '/');

    if(parts.empty()){
        return -std::ERROR_INVALID_FILE_PATH;
    }

    auto last = parts.back();
    parts.pop_back();

    //TODO file search should be done entirely by the file system

    auto content = fat32::read_file(*disks::mounted_disk(), *disks::mounted_partition(), parts, last);

    if(content.empty()){
        return 0;
    }

    size_t i = 0;
    for(; i < content.size() && i < max; ++i){
        buffer[i] = content[i];
    }

    return i;
}
