//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VFS_FILE_H
#define VFS_FILE_H

#include <datetime.hpp>
#include <types.hpp>
#include <string.hpp>

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
};

} //end of namespace vfs

#endif
