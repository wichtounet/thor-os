//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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

struct sysfs_file_system : vfs::file_system {
private:
    path mount_point;

public:
    sysfs_file_system(path mount_point);
    ~sysfs_file_system();

    size_t statfs(vfs::statfs_info& file);
    size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read);
    size_t read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read, size_t ms);
    size_t write(const path& file_path, const char* buffer, size_t count, size_t offset, size_t& written);
    size_t clear(const path& file_path, size_t count, size_t offset, size_t& written);
    size_t truncate(const path& file_path, size_t size);
    size_t get_file(const path& file_path, vfs::file& file);
    size_t ls(const path& file_path, std::vector<vfs::file>& contents);
    size_t touch(const path& file_path);
    size_t mkdir(const path& file_path);
    size_t rm(const path& file_path);
};

typedef std::string (*dynamic_fun_t)();

void set_constant_value(const path& mount_point, const path& file_path, const std::string& value);
void set_dynamic_value(const path& mount_point, const path& file_path, dynamic_fun_t fun);

void delete_value(const path& mount_point, const path& file_path);
void delete_folder(const path& mount_point, const path& file_path);

} //end of namespace sysfs

#endif
