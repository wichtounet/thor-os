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

#include "vfs/path.hpp"

namespace vfs {

enum class partition_type {
    FAT32 = 1,
    SYSFS = 2,
    DEVFS = 3,
    PROCFS = 4,
    UNKNOWN = 100
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
int64_t clear(size_t fd, size_t count, size_t offset = 0);
int64_t truncate(size_t fd, size_t size);
int64_t entries(size_t fd, char* buffer, size_t size);
int64_t mounts(char* buffer, size_t size);

int64_t mount(partition_type type, size_t mp_fd, size_t dev_fd);
int64_t mount(partition_type type, const char* mount_point, const char* device);

/*!
 * \brief Directly read a file into a std::string buffer
 *
 * This is only used directly by the kernel as an easy way to read a complete
 * file. This is not safe to be exposed from user space calls.
 *
 * \param file The file to read
 * \param content The std::string to fill with the file contents
 *
 * \return An error code if something went wrong, 0 otherwise
 */
int64_t direct_read(const path& file, std::string& content);

/*!
 * \brief Directly read a file or a device
 *
 * This is meant to be used by file system drivers.
 *
 * \param file Path to the file (or device)
 * \param buffer The output buffer
 * \param count The number of bytes to read
 * \param offset The offset where to start reading
 *
 * \return An error code if something went wrong, 0 otherwise
 */
int64_t direct_read(const path& file, char* buffer, size_t count, size_t offset = 0);

/*!
 * \brief Directly write a file or a device
 *
 * This is meant to be used by file system drivers.
 *
 * \param file Path to the file (or device)
 * \param buffer The input buffer
 * \param count The number of bytes to write
 * \param offset The offset where to start writing
 *
 * \return An error code if something went wrong, 0 otherwise
 */
int64_t direct_write(const path& file, const char* buffer, size_t count, size_t offset = 0);

} //end of namespace vfs

#endif
