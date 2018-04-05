//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <types.hpp>
#include <unique_ptr.hpp>
#include <algorithms.hpp>

#include <tlib/errors.hpp>

#include "fs/sysfs.hpp"

#include "console.hpp"
#include "assert.hpp"

namespace {

path sys_path;

struct sys_value {
    std::string name;
    std::string _value;
    sysfs::dynamic_fun_t fun           = nullptr;
    sysfs::dynamic_fun_data_t fun_data = nullptr;
    void* data                         = nullptr;

    sys_value() {}
    sys_value(std::string_view name, std::string_view value)
            : name(name.begin(), name.end()), _value(value.begin(), value.end()) {
        //Nothing else to init
    }

    sys_value(std::string_view name, sysfs::dynamic_fun_t fun)
            : name(name.begin(), name.end()), fun(fun) {
        //Nothing else to init
    }

    sys_value(std::string_view name, sysfs::dynamic_fun_data_t fun_data, void* data)
            : name(name.begin(), name.end()), fun_data(fun_data), data(data) {
        //Nothing else to init
    }

    sys_value(sys_value&) = default;
    sys_value(sys_value&&) = default;

    sys_value& operator=(sys_value&) = default;
    sys_value& operator=(sys_value&&) = default;

    std::string value() const {
        if (fun_data){
            return fun_data(data);
        } else if (fun) {
            return fun();
        } else {
            return _value;
        }
    }
};

struct sys_folder {
    std::string name;
    std::vector<sys_folder> folders;
    std::vector<sys_value> values;

    sys_folder() {}

    explicit sys_folder(std::string_view name)
            : name(name.begin(), name.end()) {
        //Nothing else to init
    }

    sys_folder(sys_folder&) = default;
    sys_folder(sys_folder&&) = default;

    sys_folder& operator=(sys_folder&) = default;
    sys_folder& operator=(sys_folder&&) = default;
};

std::vector<sys_folder> root_folders;

sys_folder& find_root_folder(const path& mount_point) {
    thor_assert(mount_point.is_sub_root(), "Unsupported mount point");

    for (auto& sys_folder : root_folders) {
        if (sys_folder.name == mount_point.sub_root_name()) {
            return sys_folder;
        }
    }

    return root_folders.emplace_back(mount_point.sub_root_name());
}

sys_folder& find_folder(sys_folder& root, const path& file_path, size_t i, size_t last){
    auto name = file_path[i];

    for (auto& folder : root.folders) {
        if (folder.name == name) {
            if (i == last - 1) {
                return folder;
            } else {
                return find_folder(folder, file_path, i + 1, last);
            }
        }
    }

    root.folders.emplace_back(name);

    if (i == last - 1) {
        return root.folders.back();
    } else {
        return find_folder(root.folders.back(), file_path, i + 1, last);
    }
}

bool exists_folder(sys_folder& root, const path& file_path, size_t i, size_t last) {
    auto name = file_path[i];

    for (auto& folder : root.folders) {
        if (folder.name == name) {
            if (i == last - 1) {
                return true;
            } else {
                return exists_folder(folder, file_path, i + 1, last);
            }
        }
    }

    return false;
}

size_t get_file(const sys_folder& folder, const path& file_path, vfs::file& f) {
    for (auto& file : folder.folders) {
        if (file.name == file_path.base_name()) {
            f.file_name = file.name;
            f.directory = true;
            f.hidden    = false;
            f.system    = false;
            f.size      = 0;

            return 0;
        }
    }

    for (auto& file : folder.values) {
        if (file.name == file_path.base_name()) {
            f.file_name = file.name;
            f.directory = false;
            f.hidden    = false;
            f.system    = false;
            f.size      = file.value().size();

            return 0;
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t ls(const sys_folder& folder, std::vector<vfs::file>& contents) {
    for (auto& file : folder.folders) {
        contents.emplace_back(file.name, true, false, false, 0UL);
    }

    for (auto& file : folder.values) {
        contents.emplace_back(file.name, false, false, false, file.value().size());
    }

    return 0;
}

size_t read(const sys_folder& folder, const path& file_path, char* buffer, size_t count, size_t offset, size_t& read) {
    for (auto& file : folder.values) {
        if (file.name == file_path.base_name()) {
            auto value = file.value();

            if (offset > value.size()) {
                return std::ERROR_INVALID_OFFSET;
            }

            read = std::min(count, value.size() - offset);
            std::copy_n(value.c_str() + offset, read, buffer);

            return 0;
        }
    }

    for (auto& file : folder.folders) {
        if (file.name == file_path.base_name()) {
            return std::ERROR_DIRECTORY;
        }
    }

    return std::ERROR_NOT_EXISTS;
}

void set_value(sys_folder& folder, std::string_view name, const std::string& value) {
    for (auto& v : folder.values) {
        if (v.name == name) {
            v._value = value;
            return;
        }
    }

    folder.values.emplace_back(name, value);
}

void set_value(sys_folder& folder, std::string_view name, sysfs::dynamic_fun_t fun) {
    for (auto& v : folder.values) {
        if (v.name == name) {
            v.fun = fun;
            return;
        }
    }

    folder.values.emplace_back(name, fun);
}

void set_value(sys_folder& folder, std::string_view name, sysfs::dynamic_fun_data_t fun, void* data) {
    for (auto& v : folder.values) {
        if (v.name == name) {
            v.fun_data = fun;
            v.data     = data;
            return;
        }
    }

    folder.values.emplace_back(name, fun, data);
}

void delete_value(sys_folder& folder, std::string_view name) {
    folder.values.erase(std::remove_if(folder.values.begin(), folder.values.end(), [&name](const sys_value& value){
        return value.name == name;
    }), folder.values.end());
}

void delete_folder(sys_folder& folder, std::string_view name) {
    folder.folders.erase(std::remove_if(folder.folders.begin(), folder.folders.end(), [&name](const sys_folder& value){
        return value.name == name;
    }), folder.folders.end());
}

} //end of anonymous namespace

sysfs::sysfs_file_system::sysfs_file_system(path mp)
        : mount_point(mp) {
    //Nothing to init
}

sysfs::sysfs_file_system::~sysfs_file_system() {
    //Nothing to delete
}

size_t sysfs::sysfs_file_system::get_file(const path& file_path, vfs::file& f) {
    auto& root_folder = find_root_folder(mount_point);

    if (file_path.is_root()) {
        f.file_name = "/";
        f.directory = true;
        f.hidden    = false;
        f.system    = false;
        f.size      = 0;

        return 0;
    } else if (file_path.size() == 2) {
        return ::get_file(root_folder, file_path, f);
    } else {
        if (exists_folder(root_folder, file_path, 1, file_path.size() - 1)) {
            auto& folder = find_folder(root_folder, file_path, 1, file_path.size() - 1);

            return ::get_file(folder, file_path, f);
        }

        return std::ERROR_NOT_EXISTS;
    }
}

size_t sysfs::sysfs_file_system::read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read) {
    auto& root_folder = find_root_folder(mount_point);

    if (file_path.is_root()) {
        return std::ERROR_DIRECTORY;
    } else if (file_path.size() == 2) {
        return ::read(root_folder, file_path, buffer, count, offset, read);
    } else {
        if (exists_folder(root_folder, file_path, 1, file_path.size() - 1)) {
            auto& folder = find_folder(root_folder, file_path, 1, file_path.size() - 1);

            return ::read(folder, file_path, buffer, count, offset, read);
        }

        return std::ERROR_NOT_EXISTS;
    }
}

size_t sysfs::sysfs_file_system::read(const path& /*file_path*/, char* /*buffer*/, size_t /*count*/, size_t /*offset*/, size_t& /*read*/, size_t /*ms*/) {
    return std::ERROR_UNSUPPORTED;
}

size_t sysfs::sysfs_file_system::write(const path&, const char*, size_t, size_t, size_t&) {
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::clear(const path&, size_t, size_t, size_t&) {
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::truncate(const path&, size_t) {
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::ls(const path& file_path, std::vector<vfs::file>& contents) {
    auto& root_folder = find_root_folder(mount_point);

    if (file_path.is_root()) {
        return ::ls(root_folder, contents);
    } else {
        if (exists_folder(root_folder, file_path, 1, file_path.size())) {
            auto& folder = find_folder(root_folder, file_path, 1, file_path.size());

            return ::ls(folder, contents);
        }

        return std::ERROR_NOT_EXISTS;
    }
}

size_t sysfs::sysfs_file_system::touch(const path&) {
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::mkdir(const path&) {
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::rm(const path&) {
    return std::ERROR_PERMISSION_DENIED;
}

size_t sysfs::sysfs_file_system::statfs(vfs::statfs_info& file) {
    file.total_size = 0;
    file.free_size  = 0;

    return 0;
}

void sysfs::set_constant_value(const path& mount_point, const path& file_path, const std::string& value) {
    auto& root_folder = find_root_folder(mount_point);

    if (file_path.size() == 2) {
        ::set_value(root_folder, file_path.base_name(), value);
    } else {
        auto& folder = find_folder(root_folder, file_path, 1, file_path.size() - 1);
        ::set_value(folder, file_path.base_name(), value);
    }
}

void sysfs::set_dynamic_value(const path& mount_point, const path& file_path, dynamic_fun_t fun) {
    auto& root_folder = find_root_folder(mount_point);

    if (file_path.size() == 2) {
        ::set_value(root_folder, file_path.base_name(), fun);
    } else {
        auto& folder = find_folder(root_folder, file_path, 1, file_path.size() - 1);
        ::set_value(folder, file_path.base_name(), fun);
    }
}

void sysfs::set_dynamic_value_data(const path& mount_point, const path& file_path, dynamic_fun_data_t fun, void* data) {
    auto& root_folder = find_root_folder(mount_point);

    if (file_path.size() == 2) {
        ::set_value(root_folder, file_path.base_name(), fun, data);
    } else {
        auto& folder = find_folder(root_folder, file_path, 1, file_path.size() - 1);
        ::set_value(folder, file_path.base_name(), fun, data);
    }
}

void sysfs::delete_value(const path& mount_point, const path& file_path) {
    auto& root_folder = find_root_folder(mount_point);

    if (file_path.size() == 2) {
        ::delete_value(root_folder, file_path.base_name());
    } else {
        auto& folder = find_folder(root_folder, file_path, 1, file_path.size() - 1);

        ::delete_value(folder, file_path.base_name());
    }
}

void sysfs::delete_folder(const path& mount_point, const path& file_path) {
    auto& root_folder = find_root_folder(mount_point);

    if (file_path.size() == 2) {
        ::delete_folder(root_folder, file_path.base_name());
    } else {
        auto& folder = find_folder(root_folder, file_path, 1, file_path.size() - 1);

        ::delete_folder(folder, file_path.base_name());
    }
}

path& sysfs::get_sys_path(){
    if(!sys_path.is_valid()){
        sys_path = "/sys";
    }

    return sys_path;
}
