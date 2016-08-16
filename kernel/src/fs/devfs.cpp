//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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
#include "logging.hpp"

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
    path mount_point;
    std::vector<device> devices;

    device_list(){};
    device_list(path mp) : mount_point(mp){}
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
    //Cannot access the root for reading
    if(file_path.is_root()){
        return std::ERROR_PERMISSION_DENIED;
    }

    for(auto& device_list : devices){
        if(device_list.mount_point == mount_point){
            for(auto& device : device_list.devices){
                if(device.name == file_path.base_name()){
                    if(!device.driver){
                        return std::ERROR_UNSUPPORTED;
                    }

                    return device.driver->read(device.data, buffer, count, offset, read);
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
                    if(!device.driver){
                        return std::ERROR_UNSUPPORTED;
                    }

                    return device.driver->write(device.data, buffer, count, offset, written);
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
                    if(!device.driver){
                        return std::ERROR_UNSUPPORTED;
                    }

                    return device.driver->clear(device.data, count, offset, written);
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

size_t devfs::devfs_file_system::statfs(statfs_info& file){
    file.total_size = 0;
    file.free_size = 0;

    return 0;
}

void devfs::register_device(const std::string& mp, const std::string& name, device_type type, dev_driver* driver, void* data){
    for(auto& device_list : devices){
        if(device_list.mount_point == mp){
            device_list.devices.emplace_back(name, type, driver, data);
            return;
        }
    }

    devices.emplace_back(mp);
    devices.back().devices.emplace_back(name, type, driver, data);
}

void devfs::deregister_device(const std::string& mp, const std::string& name){
    for(auto& device_list : devices){
        if(device_list.mount_point == mp){
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

uint64_t devfs::get_device_size(const path& device_name, size_t& size){
    if(device_name.size() != 3){
        return std::ERROR_INVALID_DEVICE;
    }

    for(auto& device_list : devices){
        if(device_list.mount_point == device_name.branch_path()){
            for(auto& device : device_list.devices){
                if(device.name == device_name.base_name()){
                    if(device.type == device_type::BLOCK_DEVICE){
                        size = device.driver->size(device.data);

                        return 0;
                    }

                    return std::ERROR_INVALID_DEVICE;
                }
            }
        }
    }

    return std::ERROR_NOT_EXISTS;;
}
