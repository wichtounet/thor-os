//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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

struct ramdisk_driver final : devfs::dev_driver {
    size_t read(void* data, char* buffer, size_t count, size_t offset, size_t& read) override;
    size_t write(void* data, const char* buffer, size_t count, size_t offset, size_t& written) override;
    size_t clear(void* data, size_t count, size_t offset, size_t& written) override;
    size_t size(void* data) override;
};

} // end of namespace ramdisk

#endif
