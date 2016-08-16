//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VFS_FILE_SYSTEM_H
#define VFS_FILE_SYSTEM_H

#include <statfs_info.hpp>
#include <vector.hpp>
#include <string.hpp>

#include "file.hpp"
#include "path.hpp"

namespace vfs {

struct file_system {
    virtual ~file_system(){};

    virtual void init(){}

    virtual size_t statfs(statfs_info& file) = 0;
    virtual size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read) = 0;
    virtual size_t write(const path& file_path, const char* buffer, size_t count, size_t offset, size_t& written) = 0;
    virtual size_t clear(const path& file_path, size_t count, size_t offset, size_t& written) = 0;
    virtual size_t truncate(const path& file_path, size_t size) = 0;
    virtual size_t get_file(const path& file_path, vfs::file& file) = 0;
    virtual size_t ls(const path& file_path, std::vector<vfs::file>& contents) = 0;
    virtual size_t touch(const path& file_path) = 0;
    virtual size_t mkdir(const path& file_path) = 0;
    virtual size_t rm(const path& file_path) = 0;
};

}

#endif
