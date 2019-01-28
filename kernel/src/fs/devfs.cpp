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

#include "fs/devfs.hpp"

#include "console.hpp"
#include "logging.hpp"

#ifdef THOR_CONFIG_DEVFS_VERBOSE
#define verbose_logf(...) logging::logf(__VA_ARGS__)
#else
#define verbose_logf(...)
#endif

namespace {

struct device {
    std::string name;
    devfs::device_type type;
    void* driver;
    void* data;

    device(){}
    device(const std::string& name, devfs::device_type type, void* driver, void* data)
        : name(name), type(type), driver(driver), data(data) {}
};

struct device_list {
    path mount_point;
    std::vector<device> devices;

    device_list(){};
    explicit device_list(path mp) : mount_point(mp){}
};

std::vector<device_list> devices;

} //end of anonymous namespace

devfs::devfs_file_system::devfs_file_system(path mp) : mount_point(mp) {
    //Nothing to init
}

devfs::devfs_file_system::~devfs_file_system(){
    //Nothing to delete
}

size_t devfs::devfs_file_system::get_file(const path& file_path, vfs::file& f){
    if(file_path.is_root()){
        f.file_name = "/";
        f.directory = true;
        f.hidden = false;
        f.system = false;
        f.size = 0;

        return 0;
    }

    //No subfolder support
    if(file_path.size() > 2){
        return std::ERROR_NOT_EXISTS;
    }

    for(auto& device_list : devices){
        if(device_list.mount_point == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.base_name()){
                    f.file_name = device.name;
                    f.directory = false;
                    f.hidden = false;
                    f.system = false;
                    f.size = 0;

                    return 0;
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t devfs::devfs_file_system::read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read){
    verbose_logf(logging::log_level::TRACE, "devfs: read(buffer=%p, count=%d, offset=%d)\n", buffer, count, offset);

    //Cannot access the root for reading
    if(file_path.is_root()){
        return std::ERROR_PERMISSION_DENIED;
    }

    for(auto& device_list : devices){
        if(device_list.mount_point == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.base_name()){
                    switch (device.type) {
                        case device_type::BLOCK_DEVICE: {
                            auto* driver = reinterpret_cast<devfs::dev_driver*>(device.driver);

                            if (!driver) {
                                return std::ERROR_UNSUPPORTED;
                            }

                            return driver->read(device.data, buffer, count, offset, read);
                        }

                        case device_type::CHAR_DEVICE: {
                            if (offset) {
                                return std::ERROR_UNSUPPORTED;
                            }

                            auto* driver = reinterpret_cast<devfs::char_driver*>(device.driver);

                            if(!driver){
                                return std::ERROR_UNSUPPORTED;
                            }

                            return driver->read(device.data, buffer, count, read);
                        }
                    }
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t devfs::devfs_file_system::read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read, size_t ms){
    //Cannot access the root for reading
    if(file_path.is_root()){
        return std::ERROR_PERMISSION_DENIED;
    }

    for(auto& device_list : devices){
        if(device_list.mount_point == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.base_name()){
                    switch (device.type) {
                        case device_type::BLOCK_DEVICE: {
                            return std::ERROR_UNSUPPORTED;
                        }

                        case device_type::CHAR_DEVICE: {
                            if (offset) {
                                return std::ERROR_UNSUPPORTED;
                            }

                            auto* driver = reinterpret_cast<devfs::char_driver*>(device.driver);

                            if(!driver){
                                return std::ERROR_UNSUPPORTED;
                            }

                            return driver->read(device.data, buffer, count, read, ms);
                        }
                    }
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t devfs::devfs_file_system::write(const path& file_path, const char* buffer, size_t count, size_t offset, size_t& written){
    //Cannot access the root for writing
    if(file_path.is_root()){
        return std::ERROR_PERMISSION_DENIED;
    }

    for(auto& device_list : devices){
        if(device_list.mount_point == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.base_name()){
                    switch (device.type) {
                        case device_type::BLOCK_DEVICE: {
                            auto* driver = reinterpret_cast<devfs::dev_driver*>(device.driver);

                            if(!driver){
                                return std::ERROR_UNSUPPORTED;
                            }

                            return driver->write(device.data, buffer, count, offset, written);
                        }

                        case device_type::CHAR_DEVICE: {
                            if (offset) {
                                return std::ERROR_UNSUPPORTED;
                            }

                            auto* driver = reinterpret_cast<devfs::char_driver*>(device.driver);

                            if(!driver){
                                return std::ERROR_UNSUPPORTED;
                            }

                            return driver->write(device.data, buffer, count, written);
                        }
                    }
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t devfs::devfs_file_system::clear(const path& file_path, size_t count, size_t offset, size_t& written){
    //Cannot access the root for writing
    if(file_path.is_root()){
        return std::ERROR_PERMISSION_DENIED;
    }

    for(auto& device_list : devices){
        if(device_list.mount_point == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.base_name()){
                    switch (device.type) {
                        case device_type::BLOCK_DEVICE: {
                            auto* driver = reinterpret_cast<devfs::dev_driver*>(device.driver);

                            if (!driver) {
                                return std::ERROR_UNSUPPORTED;
                            }

                            return driver->clear(device.data, count, offset, written);
                        }

                        case device_type::CHAR_DEVICE: {
                            return std::ERROR_UNSUPPORTED;
                        }
                    }
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t devfs::devfs_file_system::truncate(const path&, size_t){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::ls(const path& file_path, std::vector<vfs::file>& contents){
    //No subfolder support
    if(file_path.size() > 1){
        return std::ERROR_NOT_EXISTS;
    }

    for(auto& device_list : devices){
        if(device_list.mount_point == mount_point){
            for(auto& device : device_list.devices){
                vfs::file f;
                f.file_name = device.name;
                f.directory = false;
                f.hidden = false;
                f.system = false;
                f.size = 0;
                contents.emplace_back(std::move(f));
            }

            break;
        }
    }

    return 0;
}

size_t devfs::devfs_file_system::touch(const path& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::mkdir(const path& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::rm(const path& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::statfs(vfs::statfs_info& file){
    file.total_size = 0;
    file.free_size = 0;

    return 0;
}

void devfs::register_device(std::string_view mp, const std::string& name, device_type type, void* driver, void* data){
    for(auto& device_list : devices){
        if(device_list.mount_point == mp){
            device_list.devices.emplace_back(name, type, driver, data);
            return;
        }
    }

    devices.emplace_back(mp).devices.emplace_back(name, type, driver, data);
}

void devfs::deregister_device(std::string_view mp, const std::string& name){
    for(auto& device_list : devices){
        if(device_list.mount_point == mp){
            device_list.devices.erase(std::remove_if(device_list.devices.begin(), device_list.devices.end(), [&name](const device& dev){
                return dev.name == name;
            }), device_list.devices.end());

            return;
        }
    }
}

uint64_t devfs::get_device_size(const path& device_name, size_t& size){
    if(device_name.size() != 3){
        return std::ERROR_INVALID_DEVICE;
    }

    for (auto& device_list : devices) {
        if (device_list.mount_point == device_name.branch_path()) {
            for (auto& device : device_list.devices) {
                if (device.name == device_name.base_name()) {
                    switch (device.type) {
                        case device_type::BLOCK_DEVICE: {
                            auto* driver = reinterpret_cast<devfs::dev_driver*>(device.driver);

                            size = driver->size(device.data);

                            return 0;
                        }

                        case device_type::CHAR_DEVICE: {
                            return std::ERROR_INVALID_DEVICE;
                        }
                    }
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;
}
