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

#include "fs/devfs.hpp"

#include "console.hpp"

namespace {


} //end of anonymous namespace

devfs::devfs_file_system::devfs_file_system(std::string mp) : mount_point(mp) {
    //Nothing to init
}

devfs::devfs_file_system::~devfs_file_system(){
    //Nothing to delete
}

size_t devfs::devfs_file_system::get_file(const std::vector<std::string>& file_path, vfs::file& f){
    //TODO
}

size_t devfs::devfs_file_system::read(const std::vector<std::string>& file_path, char* buffer, size_t count, size_t offset, size_t& read){
    //TODO
}

size_t devfs::devfs_file_system::write(const std::vector<std::string>&, const char*, size_t, size_t, size_t&){
    //TODO
}

size_t devfs::devfs_file_system::truncate(const std::vector<std::string>& file_path, size_t size){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents){
    //TODO
}

size_t devfs::devfs_file_system::touch(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::mkdir(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::rm(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::statfs(statfs_info& file){
    file.total_size = 0;
    file.free_size = 0;

    return 0;
}

