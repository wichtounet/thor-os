//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unique_ptr.hpp>
#include <array.hpp>
#include <string.hpp>

#include "disks.hpp"
#include "ata.hpp"
#include "thor.hpp"
#include "console.hpp"

#include "fs/devfs.hpp"

namespace {

//For now, 4 is enough as only the ata driver is implemented
std::array<disks::disk_descriptor, 4> _disks;

uint64_t number_of_disks = 0;

struct partition_descriptor_t {
    uint8_t boot_flag;
    uint8_t chs_begin[3];
    uint8_t type_code;
    uint8_t chs_end[3];
    uint32_t lba_begin;
    uint32_t sectors;
} __attribute__ ((packed));

static_assert(sizeof(partition_descriptor_t) == 16, "A partition descriptor is 16 bytes long");

struct boot_record_t {
    uint8_t boot_code[446];
    partition_descriptor_t partitions[4];
    uint16_t signature;
} __attribute__ ((packed));

static_assert(sizeof(boot_record_t) == 512, "The boot record is 512 bytes long");

ata::ata_driver ata_driver_impl;
ata::ata_part_driver ata_part_driver_impl;

devfs::dev_driver* ata_driver = &ata_driver_impl;
devfs::dev_driver* ata_part_driver = &ata_part_driver_impl;
devfs::dev_driver* atapi_driver = nullptr;

} //end of anonymous namespace

void disks::detect_disks(){
    ata::detect_disks();

    char cdrom = 'a';
    char disk = 'a';

    for(uint8_t i = 0; i < ata::number_of_disks(); ++i){
        auto& descriptor = ata::drive(i);

        if(descriptor.present){
            if(descriptor.atapi){
                _disks[number_of_disks] = {number_of_disks, disks::disk_type::ATAPI, &descriptor};

                std::string name = "cd";
                name += cdrom++;

                devfs::register_device("/dev/", name, devfs::device_type::BLOCK_DEVICE, atapi_driver, &descriptor);
            } else {
                _disks[number_of_disks] = {number_of_disks, disks::disk_type::ATA, &descriptor};

                std::string name = "hd";
                name += disk++;

                devfs::register_device("/dev/", name, devfs::device_type::BLOCK_DEVICE, ata_driver, &descriptor);

                char part = '1';

                for(auto& partition : partitions(_disks[number_of_disks])){
                    auto part_name = name + part++;

                    devfs::register_device("/dev/", part_name, devfs::device_type::BLOCK_DEVICE, ata_part_driver, new partition_descriptor(partition));
                }
            }

            ++number_of_disks;
        }
    }
}

uint64_t disks::detected_disks(){
    return number_of_disks;
}

disks::disk_descriptor& disks::disk_by_index(uint64_t index){
    return _disks[index];
}

disks::disk_descriptor& disks::disk_by_uuid(uint64_t uuid){
    for(uint64_t i = 0; i < number_of_disks; ++i){
        if(_disks[i].uuid == uuid){
            return _disks[i];
        }
    }

    __builtin_unreachable();
}

bool disks::disk_exists(uint64_t uuid){
    for(uint64_t i = 0; i < number_of_disks; ++i){
        if(_disks[i].uuid == uuid){
            return true;
        }
    }

    return false;
}

const char* disks::disk_type_to_string(disk_type type){
    switch(type){
        case disk_type::ATA:
            return "ATA";
        case disk_type::ATAPI:
            return "ATAPI";
        default:
            return "Invalid Type";
    }
}

const char* disks::partition_type_to_string(vfs::partition_type type){
    switch(type){
        case vfs::partition_type::FAT32:
            return "FAT32";
        case vfs::partition_type::UNKNOWN:
            return "Unknown";
        default:
            return "Invalid Type";
    }
}

bool disks::read_sectors(const disk_descriptor& disk, uint64_t start, uint8_t count, void* destination){
    switch(disk.type){
        case disk_type::ATA:
            return ata::read_sectors(*static_cast<ata::drive_descriptor*>(disk.descriptor), start, count, destination);

        default:
            return false;
    }
}

bool disks::write_sectors(const disk_descriptor& disk, uint64_t start, uint8_t count, void* destination){
    switch(disk.type){
        case disk_type::ATA:
            return ata::write_sectors(*static_cast<ata::drive_descriptor*>(disk.descriptor), start, count, destination);

        default:
            return false;
    }
}

std::unique_heap_array<disks::partition_descriptor> disks::partitions(disk_descriptor& disk){
    std::unique_ptr<boot_record_t> boot_record(new boot_record_t());

    if(!read_sectors(disk, 0, 1, boot_record.get())){
        k_print_line("Read Boot Record failed");

        return {};
    } else {
        if(boot_record->signature != 0xAA55){
            k_print_line("Invalid boot record signature");

            return {};
        }

        uint64_t n = 0;
        for(int i = 0; i < 4; ++i){
            if(boot_record->partitions[i].type_code > 0){
                ++n;
            }
        }

        std::unique_heap_array<partition_descriptor> partitions(n);
        uint64_t p = 0;

        for(uint64_t i = 0; i < 4; ++i){
            if(boot_record->partitions[i].type_code > 0){
                vfs::partition_type type;
                if(boot_record->partitions[i].type_code == 0x0B || boot_record->partitions[i].type_code == 0x0C){
                    type = vfs::partition_type::FAT32;
                } else {
                    type = vfs::partition_type::UNKNOWN;
                }

                partitions[p] = {p, type, boot_record->partitions[i].lba_begin, boot_record->partitions[i].sectors, &disk};

                ++p;
            }
        }

        return partitions;
    }
}
