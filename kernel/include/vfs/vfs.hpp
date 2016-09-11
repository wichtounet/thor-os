//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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
    SYSFS,
    DEVFS,
    PROCFS,
    UNKNOWN
};

void init();

int64_t open(const char* file, size_t flags);
void close(size_t fd);

int64_t stat(size_t fd, stat_info& info);
int64_t statfs(const char* mount_point, statfs_info& info);
int64_t mkdir(const char* file);
int64_t rm(const char* file);
int64_t read(size_t fd, char* buffer, size_t count, size_t offset = 0);
int64_t write(size_t fd, const char* buffer, size_t count, size_t offset = 0);
int64_t truncate(size_t fd, size_t size);
int64_t entries(size_t fd, char* buffer, size_t size);
int64_t mounts(char* buffer, size_t size);

int64_t mount(partition_type type, const char* mount_point, const char* device);

//Used only inside kernel as a easy way to read a complete file
int64_t direct_read(const std::string& file, std::string& content);

//Use only inside the kernel for FS to access devices
int64_t direct_read(const char* file, char* buffer, size_t count, size_t offset = 0);
int64_t direct_write(const char* file, const char* buffer, size_t count, size_t offset = 0);

} //end of namespace vfs

#endif
