#ifndef DISKS_H
#define DISKS_H

#include "types.hpp"
#include "unique_ptr.hpp"

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

uint64_t detected_disks();

bool disk_exists(uint64_t uuid);

const disk_descriptor& disk_by_index(uint64_t index);
const disk_descriptor& disk_by_uuid(uint64_t uuid);

const char* disk_type_to_string(disk_type type);
const char* partition_type_to_string(partition_type type);

bool read_sectors(const disk_descriptor& disk, uint64_t start, uint8_t count, void* destination);

unique_ptr<partition_descriptor> partitions(const disk_descriptor& disk);

}

#endif
