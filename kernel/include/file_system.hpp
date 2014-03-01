//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VFS_FILE_SYSTEM_H
#define VFS_FILE_SYSTEM_H

#include "file.hpp"

namespace vfs {

struct file_system {
    virtual ~file_system(){};

    virtual size_t read(const std::vector<std::string>& file_path, std::string& content) = 0;
    virtual size_t ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents) = 0;
    virtual size_t touch(const std::vector<std::string>& file_path) = 0;
    virtual size_t mkdir(const std::vector<std::string>& file_path) = 0;
    virtual size_t rm(const std::vector<std::string>& file_path) = 0;
};

}

#endif
