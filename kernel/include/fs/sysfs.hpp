//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SYSFS_HPP
#define SYSFS_HPP

#include <vector.hpp>
#include <string.hpp>
#include <pair.hpp>

#include "vfs/file_system.hpp"

namespace sysfs {

struct sysfs_file_system final : vfs::file_system {
    sysfs_file_system(path mount_point);
    ~sysfs_file_system();

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

using dynamic_fun_t = std::string (*)();
using dynamic_fun_data_t = std::string (*)(void*);

void set_constant_value(const path& mount_point, const path& file_path, const std::string& value);
void set_dynamic_value(const path& mount_point, const path& file_path, dynamic_fun_t fun);
void set_dynamic_value_data(const path& mount_point, const path& file_path, dynamic_fun_data_t fun, void* data);

void delete_value(const path& mount_point, const path& file_path);
void delete_folder(const path& mount_point, const path& file_path);

path& get_sys_path();

} //end of namespace sysfs

#endif
