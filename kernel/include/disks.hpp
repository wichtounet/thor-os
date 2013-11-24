//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DISKS_H
#define DISKS_H

#include "types.hpp"
#include "array.hpp"
#include "vector.hpp"
#include "string.hpp"

namespace disks {

enum class disk_type {
    ATA
};

struct disk_descriptor {
    uint64_t uuid;
    disk_type type;
    void* descriptor;
};

enum class partition_type {
    FAT32,
    UNKNOWN
};

struct partition_descriptor {
    uint64_t uuid;
    partition_type type;
    uint64_t start;
    uint64_t sectors;
};

struct file {
    char name[11];
    bool directory;
    bool hidden;
    bool system;
    uint64_t size;
};

void detect_disks();

uint64_t detected_disks();

bool disk_exists(uint64_t uuid);

const disk_descriptor& disk_by_index(uint64_t index);
const disk_descriptor& disk_by_uuid(uint64_t uuid);

const char* disk_type_to_string(disk_type type);
const char* partition_type_to_string(partition_type type);

bool read_sectors(const disk_descriptor& disk, uint64_t start, uint8_t count, void* destination);
unique_heap_array<partition_descriptor> partitions(const disk_descriptor& disk);
bool partition_exists(const disk_descriptor& disk, uint64_t uuid);

void mount(const disk_descriptor& disk, uint64_t uuid);
void unmount();
vector<file> ls();
uint64_t free_size();

const disk_descriptor* mounted_disk();
const partition_descriptor* mounted_partition();

const string& current_directory();
void set_current_directory(const string& directory = string());

}

#endif
