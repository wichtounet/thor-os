//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VFS_H
#define VFS_H

#include <stat_info.hpp>
#include <statfs_info.hpp>

namespace vfs {

enum class partition_type {
    FAT32,
    UNKNOWN
};

void init();

int64_t open(const char* file, size_t flags);
int64_t statfs(const char* mount_point, statfs_info& info);
int64_t stat(size_t fd, stat_info& info);
int64_t mkdir(const char* file);
int64_t rm(const char* file);
int64_t read(size_t fd, char* buffer, size_t max);
int64_t direct_read(const std::string& file, std::string& content);
int64_t entries(size_t fd, char* buffer, size_t size);
int64_t mounts(char* buffer, size_t size);

int64_t mount(partition_type type, const char* mount_point, const char* device);

void close(size_t fd);

} //end of namespace vfs

#endif
