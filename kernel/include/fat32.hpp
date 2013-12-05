#ifndef FAT32_H
#define FAT32_H

#include "disks.hpp"
#include "vector.hpp"
#include "string.hpp"

namespace fat32 {

typedef const disks::disk_descriptor& dd;

uint64_t free_size(dd disk, const disks::partition_descriptor& partition);
vector<disks::file> ls(dd disk, const disks::partition_descriptor& partition, const vector<string>& path);
string read_file(dd disk, const disks::partition_descriptor& partition, const vector<string>& path, const string& file);

}

#endif
