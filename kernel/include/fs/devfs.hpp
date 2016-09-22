//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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

struct dev_driver {
    virtual size_t read(void* data, char* buffer, size_t count, size_t offset, size_t& read) = 0;
    virtual size_t write(void* data, const char* buffer, size_t count, size_t offset, size_t& written) = 0;
    virtual size_t clear(void* data, size_t count, size_t offset, size_t& written) = 0;
    virtual size_t size(void* data) = 0;
};

struct char_driver {
    virtual size_t read(void* data, char* buffer, size_t count, size_t& read) = 0;
    virtual size_t read(void* data, char* buffer, size_t count, size_t& read, size_t ms) = 0;
    virtual size_t write(void* data, const char* buffer, size_t count, size_t& written) = 0;
};

struct devfs_file_system : vfs::file_system {
private:
    path mount_point;

public:
    devfs_file_system(path mount_point);
    ~devfs_file_system();

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

void register_device(const std::string& mp, const std::string& name, device_type type, void* driver, void* data);
void deregister_device(const std::string& mp, const std::string& name);

uint64_t get_device_size(const path& device_name, size_t& size);

} //end of namespace devfs

#endif
