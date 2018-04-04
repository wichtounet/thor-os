//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef VFS_FILE_SYSTEM_H
#define VFS_FILE_SYSTEM_H

#include <vector.hpp>
#include <string.hpp>

#include <tlib/statfs_info.hpp>

#include "file.hpp"
#include "path.hpp"

namespace vfs {

/*!
 * \brief File system interface
 */
struct file_system {
    /*!
     * \brief Destroy the file system
     */
    virtual ~file_system(){};

    /*!
     * \brief Initialize the file system
     */
    virtual void init(){}

    /*!
     * \brief Query information about the file system itself
     * \return 0 on success, an error code otherwise
     */
    virtual size_t statfs(vfs::statfs_info& file) = 0;

    /*!
     * \brief Read a file
     * \param file_path The path to the file to read
     * \param buffer The buffer into which to read
     * \param count The amount of bytes to read
     * \param offset The offset at which to start reading
     * \param read output reference to indicate the number of bytes read
     * \return 0 on success, an error code otherwise
     */
    virtual size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read) = 0;

    /*!
     * \brief Read a file, waiting only the given amount of ime
     * \param file_path The path to the file to read
     * \param buffer The buffer into which to read
     * \param count The amount of bytes to read
     * \param offset The offset at which to start reading
     * \param read output reference to indicate the number of bytes read
     * \param ms The amount of time, in milliseconds, to wait for the read
     * \return 0 on success, an error code otherwise
     */
    virtual size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read, size_t ms) = 0;

    /*!
     * \brief Write to a file
     * \param file_path The path to the file to write
     * \param buffer The buffer from which to read
     * \param count The amount of bytes to write
     * \param offset The offset at which to start writing
     * \param written output reference to indicate the number of bytes written
     * \return 0 on success, an error code otherwise
     */
    virtual size_t write(const path& file_path, const char* buffer, size_t count, size_t offset, size_t& written) = 0;

    /*!
     * \brief Clear a portion of a file (write zeroes)
     * \param file_path The path to the file to write
     * \param count The amount of bytes to write
     * \param offset The offset at which to start writing
     * \param written output reference to indicate the number of bytes written
     * \return 0 on success, an error code otherwise
     */
    virtual size_t clear(const path& file_path, size_t count, size_t offset, size_t& written) = 0;

    /*!
     * \brief Change the size of a file
     * \param file_path The path to the file to modify
     * \param size The new size of the file
     * \return 0 on success, an error code otherwise
     */
    virtual size_t truncate(const path& file_path, size_t size) = 0;

    /*!
     * \brief Get informations about a file
     * \param file_path The path to the file
     * \param file The structure to fill with the information
     * \return 0 on success, an error code otherwise
     */
    virtual size_t get_file(const path& file_path, vfs::file& file) = 0;

    /*!
     * \brief Return the files inside a directory
     * \param file_path The path to the directory
     * \param contents The list of child to fill
     * \return 0 on success, an error code otherwise
     */
    virtual size_t ls(const path& file_path, std::vector<vfs::file>& contents) = 0;

    /*!
     * \brief Create the given file on the file system
     * \param file_path The path to the file to create
     * \return 0 on success, an error code otherwise
     */
    virtual size_t touch(const path& file_path) = 0;

    /*!
     * \brief Create the given directory on the file system
     * \param file_path The path to the directory to create
     * \return 0 on success, an error code otherwise
     */
    virtual size_t mkdir(const path& file_path) = 0;

    /*!
     * \brief Remove the given file from the file system
     * \param file_path The path to the file to remove
     * \return 0 on success, an error code otherwise
     */
    virtual size_t rm(const path& file_path) = 0;
};

}

#endif
