//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PROCFS_HPP
#define PROCFS_HPP

#include <vector.hpp>
#include <string.hpp>
#include <pair.hpp>

#include "process.hpp"

#include "vfs/file_system.hpp"

namespace procfs {

struct procfs_file_system final : vfs::file_system {
    procfs_file_system(path mount_point);
    ~procfs_file_system();

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

void set_pcb(const scheduler::process_control_t* pcb);

} // end of namespace procfs

#endif
