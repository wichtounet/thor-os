//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
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

#include "vfs.hpp"
#include "file.hpp"

namespace disks {

enum class disk_type {
    ATA,
    ATAPI
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
};

void detect_disks();

uint64_t detected_disks();

bool disk_exists(uint64_t uuid);

const disk_descriptor& disk_by_index(uint64_t index);
const disk_descriptor& disk_by_uuid(uint64_t uuid);

const char* disk_type_to_string(disk_type type);
const char* partition_type_to_string(vfs::partition_type type);

bool read_sectors(const disk_descriptor& disk, uint64_t start, uint8_t count, void* destination);
bool write_sectors(const disk_descriptor& disk, uint64_t start, uint8_t count, void* destination);
std::unique_heap_array<partition_descriptor> partitions(const disk_descriptor& disk);
bool partition_exists(const disk_descriptor& disk, uint64_t uuid);

}

#endif
