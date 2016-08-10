//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DISKS_H
#define DISKS_H

#include <datetime.hpp>
#include <types.hpp>
#include <array.hpp>
#include <vector.hpp>
#include <string.hpp>

#include "vfs/vfs.hpp"
#include "vfs/file.hpp"

namespace disks {

enum class disk_type {
    ATA,
    ATAPI,
    RAM
};

struct disk_descriptor {
    uint64_t uuid;
    disk_type type;
    void* descriptor;
};

struct partition_descriptor {
    uint64_t uuid;
    vfs::partition_type type;
    uint64_t start;
    uint64_t sectors;
    disk_descriptor* disk;
};

void detect_disks();

disk_descriptor& disk_by_index(uint64_t index);
disk_descriptor& disk_by_uuid(uint64_t uuid);

std::unique_heap_array<partition_descriptor> partitions(disk_descriptor& disk);

}

#endif
