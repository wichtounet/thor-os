//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DISKS_H
#define DISKS_H

#include <types.hpp>
#include <array.hpp>
#include <vector.hpp>
#include <string.hpp>

#include <tlib/datetime.hpp>

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
