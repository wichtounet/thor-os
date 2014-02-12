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
        if(f.file_name == file){
            if(f.directory){
                return -std::ERROR_DIRECTORY;
            }

            return scheduler::register_new_handle(file);
        }
    }

    return -std::ERROR_NOT_EXISTS;
}
