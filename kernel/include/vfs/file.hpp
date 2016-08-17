//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VFS_FILE_H
#define VFS_FILE_H

#include <types.hpp>
#include <string.hpp>

#include <tlib/datetime.hpp>

namespace vfs {

struct file {
    std::string file_name;
    bool directory;
    bool hidden;
    bool system;
    uint64_t size;
    datetime created;
    datetime modified;
    datetime accessed;

    //File system specific
    size_t location;
    size_t position;

    file(){};
    file(std::string file_name, bool directory, bool hidden, bool system, uint64_t size)
        : file_name(file_name), directory(directory), hidden(hidden), system(system), size(size) {};
};

} //end of namespace vfs

#endif
