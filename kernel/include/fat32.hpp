//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FAT32_H
#define FAT32_H

#include "disks.hpp"

#include "stl/vector.hpp"
#include "stl/string.hpp"

namespace fat32 {

typedef const disks::disk_descriptor& dd;

uint64_t free_size(dd disk, const disks::partition_descriptor& partition);
vector<disks::file> ls(dd disk, const disks::partition_descriptor& partition, const vector<std::string>& path);
std::string read_file(dd disk, const disks::partition_descriptor& partition, const vector<std::string>& path, const std::string& file);

}

#endif
