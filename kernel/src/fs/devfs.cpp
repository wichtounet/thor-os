//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <types.hpp>
#include <unique_ptr.hpp>
#include <algorithms.hpp>
#include <errors.hpp>

#include "fs/devfs.hpp"

#include "console.hpp"

namespace {

struct device {
    std::string name;
    devfs::device_type type;
    devfs::dev_driver* driver;
    void* data;

    device(){}
    device(std::string name, devfs::device_type type, devfs::dev_driver* driver, void* data)
        : name(name), type(type), driver(driver), data(data) {}
};

struct device_list {
    std::string name;
    std::vector<device> devices;

    device_list(){};
    device_list(std::string name) : name(name){}
};

std::vector<device_list> devices;

} //end of anonymous namespace

devfs::devfs_file_system::devfs_file_system(std::string mp) : mount_point(mp) {
    //Nothing to init
}

devfs::devfs_file_system::~devfs_file_system(){
    //Nothing to delete
}

size_t devfs::devfs_file_system::get_file(const std::vector<std::string>& file_path, vfs::file& f){
    if(file_path.empty()){
        f.file_name = "/";
        f.directory = true;
        f.hidden = false;
        f.system = false;
        f.size = 0;

        return 0;
    }

    //No subfolder support
    if(file_path.size() > 1){
        return std::ERROR_NOT_EXISTS;
    }

    for(auto& device_list : devices){
        if(device_list.name == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.back()){
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

size_t devfs::devfs_file_system::read(const std::vector<std::string>& file_path, char* buffer, size_t count, size_t offset, size_t& read){
    //Cannot access the root for reading
    if(file_path.empty()){
        return std::ERROR_PERMISSION_DENIED;
    }

    for(auto& device_list : devices){
        if(device_list.name == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.back()){
                    return device.driver->read(device.data, buffer, count, offset, read);
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t devfs::devfs_file_system::write(const std::vector<std::string>& file_path, const char* buffer, size_t count, size_t offset, size_t& written){
    //Cannot access the root for writing
    if(file_path.empty()){
        return std::ERROR_PERMISSION_DENIED;
    }

    for(auto& device_list : devices){
        if(device_list.name == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.back()){
                    return device.driver->write(device.data, buffer, count, offset, written);
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t devfs::devfs_file_system::truncate(const std::vector<std::string>&, size_t){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents){
    //No subfolder support
    if(file_path.size() > 0){
        return std::ERROR_NOT_EXISTS;
    }

    for(auto& device_list : devices){
        if(device_list.name == mount_point){
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

size_t devfs::devfs_file_system::touch(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::mkdir(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::rm(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t devfs::devfs_file_system::statfs(statfs_info& file){
    file.total_size = 0;
    file.free_size = 0;

    return 0;
}

void devfs::register_device(const std::string& mp, const std::string& name, device_type type, dev_driver* driver, void* data){
    for(auto& device_list : devices){
        if(device_list.name == mp){
            device_list.devices.emplace_back(name, type, driver, data);
            return;
        }
    }

    devices.emplace_back(mp);
    devices.back().devices.emplace_back(name, type, driver, data);
}

void devfs::deregister_device(const std::string& mp, const std::string& name){
    for(auto& device_list : devices){
        if(device_list.name == mp){
            for(size_t i = 0; i < device_list.devices.size(); ++i){
                if(device_list.devices[i].name == name){
                    device_list.devices.erase(i);
                    break;
                }
            }

            return;
        }
    }
}
