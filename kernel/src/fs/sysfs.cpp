//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <types.hpp>
#include <unique_ptr.hpp>
#include <algorithms.hpp>
#include <errors.hpp>

#include "fs/sysfs.hpp"

#include "console.hpp"
#include "rtc.hpp"

namespace {

struct sys_value {
    std::string name;
    std::string value;
};

struct sys_folder {
    std::string name;
    std::vector<sys_folder> folders;
    std::vector<sys_value> values;
};

std::vector<sys_folder> root_folders;

} //end of anonymous namespace

sysfs::sysfs_file_system::sysfs_file_system(std::string mp) : mount_point(mp) {
    //Nothing to init
}

sysfs::sysfs_file_system::~sysfs_file_system(){
    //Nothing to delete
}

size_t sysfs::sysfs_file_system::get_file(const std::vector<std::string>& file_path, vfs::file& file){
    //TODO
    return std::ERROR_NOT_EXISTS;
}

size_t sysfs::sysfs_file_system::read(const std::vector<std::string>& file_path, std::string& content){
    //TODO
    return 0;
}

size_t sysfs::sysfs_file_system::ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents){
    //TODO
    return 0;
}

size_t sysfs::sysfs_file_system::touch(const std::vector<std::string>& file_path){
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::mkdir(const std::vector<std::string>& file_path){
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::rm(const std::vector<std::string>& file_path){
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::statfs(statfs_info& file){
    file.total_size = 0;
    file.free_size = 0;

    return 0;
}

void set_value(const std::string& mount_point, const std::string& path, const std::string& value){
    //TODO
}

void delete_value(const std::string& mount_point, const std::string& path){
    //TODO
}
