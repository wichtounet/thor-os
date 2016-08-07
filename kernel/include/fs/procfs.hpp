//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PROCFS_HPP
#define PROCFS_HPP

#include <vector.hpp>
#include <string.hpp>
#include <pair.hpp>

#include "process.hpp"

#include "vfs/file_system.hpp"

namespace procfs {

struct procfs_file_system : vfs::file_system {
private:
    std::string mount_point;

public:
    procfs_file_system(std::string mount_point);
    ~procfs_file_system();

    size_t statfs(statfs_info& file);
    size_t read(const std::vector<std::string>& file_path, char* buffer, size_t count, size_t offset, size_t& read);
    size_t write(const std::vector<std::string>& file_path, const char* buffer, size_t count, size_t offset, size_t& written);
    size_t truncate(const std::vector<std::string>& file_path, size_t size);
    size_t get_file(const std::vector<std::string>& file_path, vfs::file& file);
    size_t ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents);
    size_t touch(const std::vector<std::string>& file_path);
    size_t mkdir(const std::vector<std::string>& file_path);
    size_t rm(const std::vector<std::string>& file_path);
};

void set_pcb(const scheduler::process_control_t* pcb);

} // end of namespace procfs

#endif
