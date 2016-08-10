//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef RAMDISK_H
#define RAMDISK_H

#include <types.hpp>

#include "fs/devfs.hpp"

namespace ramdisk {

struct disk_descriptor {
    uint64_t id;
    uint64_t max_size;
    uint64_t pages;
    char** allocated;
};

disk_descriptor* make_disk(uint64_t max_size);

struct ramdisk_driver : devfs::dev_driver {
    size_t read(void* data, char* buffer, size_t count, size_t offset, size_t& read);
    size_t write(void* data, const char* buffer, size_t count, size_t offset, size_t& written);
};

} // end of namespace ramdisk

#endif
