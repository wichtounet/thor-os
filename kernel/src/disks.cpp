//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <unique_ptr.hpp>
#include <array.hpp>
#include <string.hpp>

#include "disks.hpp"
#include "thor.hpp"
#include "print.hpp"
#include "logging.hpp"

// The disks implementation
#include "drivers/ata.hpp"
#include "drivers/ramdisk.hpp"

#include "fs/devfs.hpp"
#include "fs/sysfs.hpp"

namespace {

//For now, 5 is enough as only the ata driver and ramdisk are implemented
std::array<disks::disk_descriptor, 5> _disks;

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
ramdisk::ramdisk_driver ramdisk_driver_impl;

devfs::dev_driver* ata_driver = &ata_driver_impl;
devfs::dev_driver* ata_part_driver = &ata_part_driver_impl;
devfs::dev_driver* ramdisk_driver = &ramdisk_driver_impl;
devfs::dev_driver* atapi_driver = nullptr;

void make_ram_disk(){
    auto* descriptor = ramdisk::make_disk(1024 * 1024); //1MiB

    if(!descriptor){
        logging::logf(logging::log_level::ERROR, "disks: failed to create /dev/ram0");
        return;
    }

    _disks[number_of_disks] = {number_of_disks, disks::disk_type::RAM, descriptor};

    devfs::register_device("/dev/", "ram0", devfs::device_type::BLOCK_DEVICE, ramdisk_driver, &_disks[number_of_disks]);

    ++number_of_disks;
}

} //end of anonymous namespace

void disks::detect_disks(){
    ata::detect_disks();

    char cdrom = 'a';
    char disk = 'a';

    for(uint8_t i = 0; i < ata::number_of_disks(); ++i){
        auto& descriptor = ata::drive(i);

        if(descriptor.present){
            std::string name;
            if(descriptor.atapi){
                _disks[number_of_disks] = {number_of_disks, disks::disk_type::ATAPI, &descriptor};

                name = "cd";
                name += cdrom++;

                devfs::register_device("/dev/", name, devfs::device_type::BLOCK_DEVICE, atapi_driver, &_disks[number_of_disks]);
            } else {
                _disks[number_of_disks] = {number_of_disks, disks::disk_type::ATA, &descriptor};

                name = "hd";
                name += disk++;

                devfs::register_device("/dev/", name, devfs::device_type::BLOCK_DEVICE, ata_driver, &_disks[number_of_disks]);

                char part = '1';

                for(auto& partition : partitions(_disks[number_of_disks])){
                    auto part_name = name + part++;

                    devfs::register_device("/dev/", part_name, devfs::device_type::BLOCK_DEVICE, ata_part_driver, new partition_descriptor(partition));
                }
            }

            sysfs::set_constant_value(sysfs::get_sys_path(), path("/ata") / name / "model", descriptor.model);
            sysfs::set_constant_value(sysfs::get_sys_path(), path("/ata") / name / "serial", descriptor.serial);
            sysfs::set_constant_value(sysfs::get_sys_path(), path("/ata") / name / "firmware", descriptor.firmware);

            ++number_of_disks;
        }
    }

    make_ram_disk();
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

std::unique_heap_array<disks::partition_descriptor> disks::partitions(disk_descriptor& disk){
    // A RAM disk has no partition
    if(disk.type == disk_type::RAM){
        return {};
    }

    auto boot_record = std::make_unique<boot_record_t>();

    size_t read;
    if(ata::read_sectors(*static_cast<ata::drive_descriptor*>(disk.descriptor), 0, 1, boot_record.get(), read) > 0){
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
