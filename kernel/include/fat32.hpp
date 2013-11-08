#ifndef FAT32_H
#define FAT32_H

#include "disks.hpp"

namespace fat32 {

void ls(const disks::disk_descriptor& disk, const disks::partition_descriptor& partition);

}

#endif
