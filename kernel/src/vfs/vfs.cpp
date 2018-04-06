//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <string.hpp>
#include <algorithms.hpp>

#include <tlib/directory_entry.hpp>
#include <tlib/mount_point.hpp>
#include <tlib/errors.hpp>
#include <tlib/flags.hpp>

#include "vfs/vfs.hpp"
#include "vfs/file_system.hpp"

#include "fs/fat32.hpp"
#include "fs/sysfs.hpp"
#include "fs/devfs.hpp"
#include "fs/procfs.hpp"

#include "scheduler.hpp"
#include "console.hpp"
#include "logging.hpp"
#include "assert.hpp"

namespace {

struct mounted_fs {
    vfs::partition_type fs_type;
    path device;
    path mount_point;
    vfs::file_system* file_system;

    mounted_fs() = default;

    mounted_fs(vfs::partition_type type, path dev, path mp, vfs::file_system* fs)
            : fs_type(type), device(dev), mount_point(mp), file_system(fs) {
        // Nothing else to init
    }
};

std::string partition_type_to_string(vfs::partition_type type) {
    switch (type) {
        case vfs::partition_type::FAT32:
            return "FAT32";
        case vfs::partition_type::SYSFS:
            return "sysfs";
        case vfs::partition_type::DEVFS:
            return "devfs";
        case vfs::partition_type::PROCFS:
            return "procfs";
        case vfs::partition_type::UNKNOWN:
            return "Unknown";
        default:
            return "Invalid Type";
    }
}

std::vector<mounted_fs> mount_point_list;

void mount_root() {
    //TODO Get information about the root from a configuration file
    mount(vfs::partition_type::FAT32, "/", "/dev/hda1");
}

void mount_sys() {
    mount(vfs::partition_type::SYSFS, "/sys/", "none");
}

void mount_dev() {
    mount(vfs::partition_type::DEVFS, "/dev/", "none");
}

void mount_proc() {
    mount(vfs::partition_type::PROCFS, "/proc/", "none");
}

path get_path(const char* file_path) {
    path p(file_path);

    if (!p.is_valid()) {
        return p;
    }

    if (p.is_relative()) {
        return scheduler::get_working_directory() / p;
    }

    return p;
}

mounted_fs& get_fs(const path& base_path) {
    size_t best       = 0;
    size_t best_match = 0;

    if (base_path.is_root()) {
        for (auto& mp : mount_point_list) {
            if (mp.mount_point.is_root()) {
                return mp;
            }
        }
    }

    for (size_t i = 0; i < mount_point_list.size(); ++i) {
        auto& mp = mount_point_list[i];

        bool match = true;
        for (size_t j = 0; j < mp.mount_point.size() && j < base_path.size(); ++j) {
            if (mp.mount_point[j] != base_path[j]) {
                match = false;
                break;
            }
        }

        if (match && mp.mount_point.size() > best) {
            best       = mp.mount_point.size();
            best_match = i;
        }
    }

    return mount_point_list[best_match];
}

path get_fs_path(const path& base_path, const mounted_fs& fs) {
    thor_assert(base_path.is_absolute(), "Invalid base_path in get_fs_path");
    thor_assert(fs.mount_point.is_absolute(), "Invalid base_path in get_fs_path");

    if (base_path == fs.mount_point) {
        return path("/");
    }

    return path("/") / base_path.sub_path(fs.mount_point.size());
}

vfs::file_system* get_new_fs(vfs::partition_type type, const path& mount_point, const path& device) {
    switch (type) {
        case vfs::partition_type::FAT32:
            return new fat32::fat32_file_system(mount_point, device);

        case vfs::partition_type::SYSFS:
            return new sysfs::sysfs_file_system(mount_point);

        case vfs::partition_type::DEVFS:
            return new devfs::devfs_file_system(mount_point);

        case vfs::partition_type::PROCFS:
            return new procfs::procfs_file_system(mount_point);

        default:
            return nullptr;
    }
}

} //end of anonymous namespace

void vfs::init() {
    mount_root();
    mount_sys();
    mount_dev();
    mount_proc();

    //Finish initilization of the file systems
    for (auto& mp : mount_point_list) {
        mp.file_system->init();
    }
}

std::expected<void> vfs::mount(partition_type type, fd_t mp_fd, fd_t dev_fd) {
    if (!scheduler::has_handle(mp_fd)) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    if (!scheduler::has_handle(dev_fd)) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    auto& mp_path  = scheduler::get_handle(mp_fd);
    auto& dev_path = scheduler::get_handle(dev_fd);

    for (auto& m : mount_point_list) {
        if (m.mount_point == mp_path) {
            return std::make_unexpected<void>(std::ERROR_ALREADY_MOUNTED);
        }
    }

    auto fs = get_new_fs(type, mp_path, dev_path);

    if (!fs) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_SYSTEM);
    }

    mount_point_list.emplace_back(type, dev_path, mp_path, fs);
    fs->init();

    auto dev_path_string = dev_path.string();
    auto mp_path_string = mp_path.string();

    logging::logf(logging::log_level::TRACE, "vfs: mounted file system %.*s at %.*s\n",
        dev_path_string.size(), dev_path_string.data(),
        mp_path_string.size(), mp_path_string.data());

    return {};
}

std::expected<void> vfs::mount(partition_type type, const char* mount_point, const char* device) {
    path mp_path(mount_point);
    path dev_path(device);

    auto fs = get_new_fs(type, mp_path, dev_path);

    if (!fs) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_SYSTEM);
    }

    mount_point_list.emplace_back(type, dev_path, mp_path, fs);

    auto dev_path_string = dev_path.string();
    auto mp_path_string = mp_path.string();

    logging::logf(logging::log_level::TRACE, "vfs: mounted file system %.*s at %.*s\n",
        dev_path_string.size(), dev_path_string.data(),
        mp_path_string.size(), mp_path_string.data());

    return {};
}

std::expected<void> vfs::statfs(const char* mount_point, vfs::statfs_info& info) {
    auto base_path = get_path(mount_point);

    if (!base_path.is_valid()) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs   = get_fs(base_path);
    auto error = fs.file_system->statfs(info);

    return std::make_expected_zero(error);
}

std::expected<vfs::fd_t> vfs::open(const char* file_path, size_t flags) {
    auto base_path = get_path(file_path);

    if (!base_path.is_valid()) {
        return std::make_unexpected<fd_t>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    //Special handling for opening the root
    if (fs_path.is_root()) {
        return scheduler::register_new_handle(base_path);
    }

    int64_t sub_result;
    if (flags & std::OPEN_CREATE) {
        vfs::file file;
        sub_result = fs.file_system->get_file(fs_path, file);

        if (sub_result == std::ERROR_NOT_EXISTS) {
            sub_result = fs.file_system->touch(fs_path);
        }
    } else {
        vfs::file file;
        sub_result = fs.file_system->get_file(fs_path, file);
    }

    if (sub_result > 0) {
        return std::make_unexpected<fd_t, size_t>(sub_result);
    } else {
        return scheduler::register_new_handle(base_path);
    }
}

void vfs::close(fd_t fd) {
    if (scheduler::has_handle(fd)) {
        scheduler::release_handle(fd);
    }
}

std::expected<void> vfs::mkdir(const char* file_path) {
    auto base_path = get_path(file_path);

    if (!base_path.is_valid()) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

#ifdef THOR_CONFIG_DEBUG_VFS
    logging::logf(logging::log_level::TRACE, "vfs: mkdir: %s \n", file_path);

    for (auto& p : base_path) {
        logging::logf(logging::log_level::TRACE, "vfs: mkdir base_path: %s\n", p.c_str());
    }

    for (auto& p : fs_path) {
        logging::logf(logging::log_level::TRACE, "vfs: mkdir fs_path: %s\n", p.c_str());
    }
#endif

    auto error = fs.file_system->mkdir(fs_path);
    return std::make_expected_zero(error);
}

std::expected<void> vfs::rm(const char* file_path) {
    auto base_path = get_path(file_path);

    if (!base_path.is_valid()) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    auto error = fs.file_system->rm(fs_path);
    return std::make_expected_zero(error);
}

std::expected<void> vfs::stat(fd_t fd, vfs::stat_info& info) {
    if (!scheduler::has_handle(fd)) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    auto& base_path = scheduler::get_handle(fd);
    auto& fs        = get_fs(base_path);
    auto fs_path    = get_fs_path(base_path, fs);

    //Special handling for root
    if (fs_path.is_root()) {
        //TODO Add file system support for stat of the root directory
        info.size  = 4096;
        info.flags = vfs::STAT_FLAG_DIRECTORY;

        return {};
    }

    vfs::file f;
    auto result = fs.file_system->get_file(fs_path, f);

    if (result) {
        return std::make_unexpected<void>(result);
    }

    info.size  = f.size;
    info.flags = 0;

    if (f.directory) {
        info.flags |= vfs::STAT_FLAG_DIRECTORY;
    }

    if (f.system) {
        info.flags |= vfs::STAT_FLAG_SYSTEM;
    }

    if (f.hidden) {
        info.flags |= vfs::STAT_FLAG_HIDDEN;
    }

    // All files starting with a .dot are hidden by default
    if (fs_path.base_name()[0] == '.') {
        info.flags |= vfs::STAT_FLAG_HIDDEN;
    }

    info.created  = f.created;
    info.modified = f.modified;
    info.accessed = f.accessed;

    return {};
}

std::expected<size_t> vfs::read(fd_t fd, char* buffer, size_t count, size_t offset) {
    if (!scheduler::has_handle(fd)) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    auto& base_path = scheduler::get_handle(fd);

    if (base_path.is_root()) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    size_t read = 0;
    auto result = fs.file_system->read(fs_path, buffer, count, offset, read);

    if (result) {
        return std::make_unexpected<size_t>(result);
    } else {
        return read;
    }
}

std::expected<size_t> vfs::read(fd_t fd, char* buffer, size_t count, size_t offset, size_t ms) {
    if (!scheduler::has_handle(fd)) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    auto& base_path = scheduler::get_handle(fd);

    if (base_path.is_root()) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    size_t read = 0;
    auto result = fs.file_system->read(fs_path, buffer, count, offset, read, ms);

    if (result) {
        return std::make_unexpected<size_t>(result);
    } else {
        return read;
    }
}

std::expected<size_t> vfs::direct_read(const path& base_path, char* buffer, size_t count, size_t offset) {
    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    size_t read = 0;
    auto result = fs.file_system->read(fs_path, buffer, count, offset, read);

    if (result) {
        return std::make_unexpected<size_t>(result);
    } else {
        return read;
    }
}

std::expected<size_t> vfs::write(fd_t fd, const char* buffer, size_t count, size_t offset) {
    if (!scheduler::has_handle(fd)) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    auto& base_path = scheduler::get_handle(fd);

    if (base_path.is_root()) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    size_t written = 0;
    auto result    = fs.file_system->write(fs_path, buffer, count, offset, written);

    if (result) {
        return std::make_unexpected<size_t>(result);
    } else {
        return written;
    }
}

std::expected<size_t> vfs::clear(fd_t fd, size_t count, size_t offset) {
    if (!scheduler::has_handle(fd)) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    auto& base_path = scheduler::get_handle(fd);

    if (base_path.is_root()) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    size_t written = 0;
    auto result    = fs.file_system->clear(fs_path, count, offset, written);

    if (result) {
        return std::make_unexpected<size_t>(result);
    } else {
        return written;
    }
}

std::expected<size_t> vfs::direct_write(const path& base_path, const char* buffer, size_t count, size_t offset) {
    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    size_t written = 0;
    auto result    = fs.file_system->write(fs_path, buffer, count, offset, written);

    if (result) {
        return std::make_unexpected<size_t>(result);
    } else {
        return written;
    }
}

std::expected<void> vfs::truncate(fd_t fd, size_t size) {
    if (!scheduler::has_handle(fd)) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    auto& base_path = scheduler::get_handle(fd);

    if (base_path.is_root()) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_PATH);
    }

    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    auto result = fs.file_system->truncate(fs_path, size);
    return std::make_expected_zero(result);
}

std::expected<size_t> vfs::direct_read(const path& base_path, std::string& content) {
    auto& fs     = get_fs(base_path);
    auto fs_path = get_fs_path(base_path, fs);

    vfs::file f;
    auto result = fs.file_system->get_file(fs_path, f);

    if (result > 0) {
        return -result;
    }

    content.reserve(f.size + 1);

    size_t read = 0;
    result      = fs.file_system->read(fs_path, content.c_str(), f.size, 0, read);

    if (result > 0) {
        return -result;
    }

    content[read] = '\0';
    content.adjust_size(read);

    if (result) {
        return std::make_unexpected<size_t>(result);
    } else {
        return read;
    }
}

std::expected<size_t> vfs::entries(fd_t fd, char* buffer, size_t size) {
    if (!scheduler::has_handle(fd)) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    auto& base_path = scheduler::get_handle(fd);
    auto& fs        = get_fs(base_path);
    auto fs_path    = get_fs_path(base_path, fs);

    std::vector<vfs::file> files;
    auto result = fs.file_system->ls(fs_path, files);

    if (result > 0) {
        return -result;
    }

    size_t total_size = 0;

    for (auto& f : files) {
        total_size += sizeof(vfs::directory_entry) + f.file_name.size();
    }

    if (size < total_size) {
        return std::make_unexpected<size_t>(std::ERROR_BUFFER_SMALL);
    }

    size_t position = 0;

    for (size_t i = 0; i < files.size(); ++i) {
        auto& file = files[i];

        auto entry = reinterpret_cast<vfs::directory_entry*>(buffer + position);

        entry->type   = 0; //TODO Fill that
        entry->length = file.file_name.size();

        if (i + 1 < files.size()) {
            entry->offset_next = file.file_name.size() + 1 + 3 * 8;
            position += entry->offset_next;
        } else {
            entry->offset_next = 0;
        }

        char* name_buffer = &(entry->name);
        std::copy(file.file_name.begin(), file.file_name.end(), name_buffer);
        name_buffer[file.file_name.size()] = '\0';
    }

    return total_size;
}

std::expected<size_t> vfs::mounts(char* buffer, size_t size) {
    size_t total_size = 0;

    for (auto& mp : mount_point_list) {
        total_size += 4 * sizeof(size_t) + 3 + mp.device.string().size() + mp.mount_point.string().size() + partition_type_to_string(mp.fs_type).size();
    }

    if (size < total_size) {
        return std::make_unexpected<size_t>(std::ERROR_BUFFER_SMALL);
    }

    size_t position = 0;

    for (size_t i = 0; i < mount_point_list.size(); ++i) {
        auto& mp = mount_point_list[i];

        auto entry = reinterpret_cast<vfs::mount_point*>(buffer + position);

        auto fs_type = partition_type_to_string(mp.fs_type);

        entry->length_mp   = mp.mount_point.string().size();
        entry->length_dev  = mp.device.string().size();
        entry->length_type = fs_type.size();

        if (i + 1 < mount_point_list.size()) {
            entry->offset_next = 4 * sizeof(size_t) + 3 + mp.device.string().size() + mp.mount_point.string().size() + fs_type.size();
            position += entry->offset_next;
        } else {
            entry->offset_next = 0;
        }

        char* name_buffer = &(entry->name);
        size_t str_pos    = 0;

        auto mount_point = mp.mount_point.string();
        for (size_t j = 0; j < mount_point.size(); ++j) {
            name_buffer[str_pos++] = mount_point[j];
        }
        name_buffer[str_pos++] = '\0';
        auto device = mp.device.string();
        for (size_t j = 0; j < device.size(); ++j) {
            name_buffer[str_pos++] = device[j];
        }
        name_buffer[str_pos++] = '\0';
        for (size_t j = 0; j < fs_type.size(); ++j) {
            name_buffer[str_pos++] = fs_type[j];
        }
        name_buffer[str_pos++] = '\0';
    }

    return total_size;
}
