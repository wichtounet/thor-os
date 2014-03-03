//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
    std::string mount_point;

public:
    sysfs_file_system(std::string mount_point);
    ~sysfs_file_system();

    size_t statfs(statfs_info& file);
    size_t read(const std::vector<std::string>& file_path, char* buffer, size_t count, size_t offset, size_t& read);
    size_t get_file(const std::vector<std::string>& file_path, vfs::file& file);
    size_t ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents);
    size_t touch(const std::vector<std::string>& file_path);
    size_t mkdir(const std::vector<std::string>& file_path);
    size_t rm(const std::vector<std::string>& file_path);
};

typedef std::string (*dynamic_fun_t)();

void set_constant_value(const std::string& mount_point, const std::string& path, const std::string& value);
void set_dynamic_value(const std::string& mount_point, const std::string& path, dynamic_fun_t fun);

void delete_value(const std::string& mount_point, const std::string& path);
void delete_folder(const std::string& mount_point, const std::string& path);

}

#endif
