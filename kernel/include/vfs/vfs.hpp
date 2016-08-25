//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VFS_H
#define VFS_H

#include <tlib/stat_info.hpp>
#include <tlib/statfs_info.hpp>

#include "vfs/path.hpp"

namespace vfs {

/*!
 * \brief Enumeration for all supported partition types
 */
enum class partition_type {
    FAT32 = 1, ///< FAT32
    SYSFS = 2, ///< Sysfs
    DEVFS = 3, ///< Devfs
    PROCFS = 4, ///< Procfs
    UNKNOWN = 100 ///< Unknown file system
};

/*!
 * \brief Init the virtual file system
 */
void init();

/*!
 * \brief Open the given file with the given flags
 * \param file The path to the file to open
 * \param flags The opening flags
 * \return The file descriptor on success, a negative error code otherwise
 */
int64_t open(const char* file, size_t flags);

/*!
 * \brief Close the given file descriptor
 */
void close(size_t fd);

/*!
 * \brief Returns information about the file represented by the fd
 * \param fd The file descriptor
 * \param info The info to fill
 * \return a status code
 */
int64_t stat(size_t fd, tlib::stat_info& info);

/*!
 * \brief Returns information about the file system
 * \param mount_point The mount point
 * \param info The info to fill
 * \return a status code
 */
int64_t statfs(const char* mount_point, tlib::statfs_info& info);

/*!
 * \brief Create a new directory
 * \param file Path to the directory to create
 * \return a status code
 */
int64_t mkdir(const char* file);

/*!
 * \brief Remove a file
 * \param file Path to the file to remove
 * \return a status code
 */
int64_t rm(const char* file);

/*!
 * \brief Read from a file
 * \param fd The file descriptor to the file
 * \param buffer The buffer to write to
 * \param count The number of bytes to read
 * \param offset The index where to start reading the file
 * \return a status code
 */
int64_t read(size_t fd, char* buffer, size_t count, size_t offset = 0);

/*!
 * \brief Write to a file
 * \param fd The file descriptor to the file
 * \param buffer The buffer to read from
 * \param count The number of bytes to write
 * \param offset The index where to start writting the file
 * \return a status code
 */
int64_t write(size_t fd, const char* buffer, size_t count, size_t offset = 0);

/*!
 * \brief Clear parts of a file content
 * \param fd The file descriptor to the file
 * \param count The number of bytes to write
 * \param offset The index where to start writting the file
 * \return a status code
 */
int64_t clear(size_t fd, size_t count, size_t offset = 0);

/*!
 * \brief Truncate the size of a file
 * \param fd The file descriptor
 * \param size The new size
 * \return a status code
 */
int64_t truncate(size_t fd, size_t size);

/*!
 * \brief List entries in the given directory
 * \param fd The file descriptor
 * \param buffer The buffer to fill with the entries
 * \param size The maximum size of the buffer
 * \return a status code
 */
int64_t entries(size_t fd, char* buffer, size_t size);

/*!
 * \brief List mounted file systems.
 * \param buffer The buffer to fill with the entries
 * \param size The maximum size of the buffer
 * \return a status code
 */
int64_t mounts(char* buffer, size_t size);

/*!
 * \brief Mount a new partition
 * \param type The type of partition
 * \param mp_fd Mount point file descriptor
 * \param dev_fd Device file descriptor
 * \return a status code
 */
int64_t mount(partition_type type, size_t mp_fd, size_t dev_fd);

/*!
 * \brief Mount a new partition
 *
 * This function is intended for direct mount from the OS code.
 *
 * \param type The type of partition
 * \param mount_point Mount point path
 * \param device Device path
 * \return a status code
 */
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
