//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
    rtc::datetime created;
    rtc::datetime modified;
    rtc::datetime accessed;

    //File system specific
    size_t location;
    size_t position;

    file(){};
    file(std::string file_name, bool directory, bool hidden, bool system, uint64_t size)
        : file_name(file_name), directory(directory), hidden(hidden), system(system), size(size) {};
};

} //end of namespace vfs

#endif
