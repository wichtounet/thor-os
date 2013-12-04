#ifndef FAT32_H
#define FAT32_H

#include "disks.hpp"
#include "vector.hpp"
#include "string.hpp"

namespace fat32 {

typedef const disks::disk_descriptor& dd;

vector<disks::file> ls(dd disk, const disks::partition_descriptor& partition, const vector<string>& path);
uint64_t free_size(dd disk, const disks::partition_descriptor& partition);

}

#endif
