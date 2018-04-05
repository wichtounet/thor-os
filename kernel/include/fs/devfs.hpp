//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DEVFS_HPP
#define DEVFS_HPP

#include <vector.hpp>
#include <string.hpp>
#include <pair.hpp>

#include "vfs/file_system.hpp"

namespace devfs {

enum class device_type {
    BLOCK_DEVICE,
    CHAR_DEVICE
};

/*!
 * \brief Block device driver interface
 */
struct dev_driver {
    /*!
     * \brief Read a block of data
     * \param data The driver data
     * \param buffer The buffer into which to read
     * \param count The amount of bytes to read
     * \param offset The offset at which to start reading
     * \param read output reference to indicate the number of bytes read
     * \return 0 on success, an error code otherwise
     */
    virtual size_t read(void* data, char* buffer, size_t count, size_t offset, size_t& read) = 0;

    /*!
     * \brief Write a block of data
     * \param data The driver data
     * \param buffer The buffer from which to read
     * \param count The amount of bytes to write
     * \param offset The offset at which to start writing
     * \param written output reference to indicate the number of bytes written
     * \return 0 on success, an error code otherwise
     */
    virtual size_t write(void* data, const char* buffer, size_t count, size_t offset, size_t& written) = 0;

    /*!
     * \brief Clear a portion of a file (write zeroes)
     * \param data The driver data
     * \param count The amount of bytes to write
     * \param offset The offset at which to start writing
     * \param written output reference to indicate the number of bytes written
     * \return 0 on success, an error code otherwise
     */
    virtual size_t clear(void* data, size_t count, size_t offset, size_t& written) = 0;

    /*!
     * \brief Return the size of the device
     * \param data The driver data
     * \return The size of the device
     */
    virtual size_t size(void* data) = 0;
};

/*!
 * \brief Character device driver interface
 */
struct char_driver {
    /*!
     * \brief Read a block of data
     * \param data The driver data
     * \param buffer The buffer into which to read
     * \param count The amount of bytes to read
     * \param read output reference to indicate the number of bytes read
     * \return 0 on success, an error code otherwise
     */
    virtual size_t read(void* data, char* buffer, size_t count, size_t& read) = 0;

    /*!
     * \brief Read a block of data only waiting for a given amount of time
     * \param data The driver data
     * \param buffer The buffer into which to read
     * \param count The amount of bytes to read
     * \param read output reference to indicate the number of bytes read
     * \param ms The amount of time, in milliseconds, to wait for the read
     * \return 0 on success, an error code otherwise
     */
    virtual size_t read(void* data, char* buffer, size_t count, size_t& read, size_t ms) = 0;

    /*!
     * \brief Write a block of data
     * \param data The driver data
     * \param buffer The buffer from which to read
     * \param count The amount of bytes to write
     * \param written output reference to indicate the number of bytes written
     * \return 0 on success, an error code otherwise
     */
    virtual size_t write(void* data, const char* buffer, size_t count, size_t& written) = 0;
};

struct devfs_file_system final : vfs::file_system {
    devfs_file_system(path mount_point);
    ~devfs_file_system();

    /*!
     * \copydoc vfs::file_system::statfs
     */
    size_t statfs(vfs::statfs_info& file) override;

    /*!
     * \copydoc vfs::file_system::read
     */
    size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read) override;

    /*!
     * \copydoc vfs::file_system::read
     */
    size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read, size_t ms) override;

    /*!
     * \copydoc vfs::file_system::write
     */
    size_t write(const path& file_path, const char* buffer, size_t count, size_t offset, size_t& written) override;

    /*!
     * \copydoc vfs::file_system::clear
     */
    size_t clear(const path& file_path, size_t count, size_t offset, size_t& written) override;

    /*!
     * \copydoc vfs::file_system::truncate
     */
    size_t truncate(const path& file_path, size_t size) override;

    /*!
     * \copydoc vfs::file_system::get_file
     */
    size_t get_file(const path& file_path, vfs::file& file) override;

    /*!
     * \copydoc vfs::file_system::ls
     */
    size_t ls(const path& file_path, std::vector<vfs::file>& contents) override;

    /*!
     * \copydoc vfs::file_system::touch
     */
    size_t touch(const path& file_path) override;

    /*!
     * \copydoc vfs::file_system::mkdir
     */
    size_t mkdir(const path& file_path) override;

    /*!
     * \copydoc vfs::file_system::rm
     */
    size_t rm(const path& file_path) override;

private:
    path mount_point;
};

void register_device(std::string_view mp, const std::string& name, device_type type, void* driver, void* data);
void deregister_device(std::string_view mp, const std::string& name);

uint64_t get_device_size(const path& device_name, size_t& size);

} //end of namespace devfs

#endif
