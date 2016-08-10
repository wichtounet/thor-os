//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ramdisk.hpp"
#include "errors.hpp"
#include "disks.hpp"

namespace {

constexpr const size_t MAX_RAMDISK = 3;

size_t current = 0;
ramdisk::disk_descriptor ramdisks[MAX_RAMDISK];

} //end of anonymous namespace

ramdisk::disk_descriptor* ramdisk::make_disk(){
    if(current == MAX_RAMDISK){
        return nullptr;
    }

    ramdisks[current].id = current;

    //TODO

    ++current;
    return &ramdisks[current - 1];
}

size_t ramdisk::ramdisk_driver::read(void* data, char* destination, size_t count, size_t offset, size_t& read){
    read = 0;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);

    return std::ERROR_INVALID_COUNT;
}

size_t ramdisk::ramdisk_driver::write(void* data, const char* source, size_t count, size_t offset, size_t& written){
    written = 0;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);

    return std::ERROR_INVALID_COUNT;
}
