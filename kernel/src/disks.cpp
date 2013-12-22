//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "disks.hpp"
#include "ata.hpp"
#include "thor.hpp"
#include "console.hpp"
#include "fat32.hpp"

#include "stl/unique_ptr.hpp"
#include "stl/array.hpp"
#include "stl/string.hpp"

namespace {

//For now, 4 is enough as only the ata driver is implemented
array<disks::disk_descriptor, 4> _disks;

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

const disks::disk_descriptor* _mounted_disk;
const disks::partition_descriptor* _mounted_partition;

vector<std::string> pwd;

} //end of anonymous namespace

void disks::detect_disks(){
    ata::detect_disks();

    for(uint8_t i = 0; i < ata::number_of_disks(); ++i){
        auto& descriptor = ata::drive(i);

        if(descriptor.present){
            _disks[number_of_disks] = {number_of_disks, disks::disk_type::ATA, &descriptor};
            ++number_of_disks;
        }
    }

    _mounted_disk = nullptr;
    _mounted_partition = nullptr;
}

uint64_t disks::detected_disks(){
    return number_of_disks;
}

const disks::disk_descriptor& disks::disk_by_index(uint64_t index){
    return _disks[index];
}

const disks::disk_descriptor& disks::disk_by_uuid(uint64_t uuid){
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
        default:
            return "Invalid Type";
    }
}

const char* disks::partition_type_to_string(partition_type type){
    switch(type){
        case partition_type::FAT32:
            return "FAT32";
        case partition_type::UNKNOWN:
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

unique_heap_array<disks::partition_descriptor> disks::partitions(const disk_descriptor& disk){
    unique_ptr<boot_record_t> boot_record(new boot_record_t());

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

        unique_heap_array<partition_descriptor> partitions(n);
        uint64_t p = 0;

        for(uint64_t i = 0; i < 4; ++i){
            if(boot_record->partitions[i].type_code > 0){
                partition_type type;
                if(boot_record->partitions[i].type_code == 0x0B || boot_record->partitions[i].type_code == 0x0C){
                    type = partition_type::FAT32;
                } else {
                    type = partition_type::UNKNOWN;
                }

                partitions[p] = {p, type, boot_record->partitions[i].lba_begin, boot_record->partitions[i].sectors};

                ++p;
            }
        }

        return partitions;
    }
}

bool disks::partition_exists(const disk_descriptor& disk, uint64_t uuid){
    for(auto& partition : partitions(disk)){
        if(partition.uuid == uuid){
            return true;
        }
    }

    return false;
}

void disks::mount(const disk_descriptor& disk, uint64_t uuid){
    _mounted_disk = &disk;

    if(_mounted_partition){
        delete _mounted_partition;
    }

    for(auto& partition : partitions(disk)){
        if(partition.uuid == uuid){
            auto p = new partition_descriptor();
            *p = partition;
            _mounted_partition = p;
            break;
        }
    }
}

void disks::unmount(){
    _mounted_disk = nullptr;

    if(_mounted_partition){
        delete _mounted_partition;
    }

    _mounted_partition = nullptr;
}

const disks::disk_descriptor* disks::mounted_disk(){
    return _mounted_disk;
}

const disks::partition_descriptor* disks::mounted_partition(){
    return _mounted_partition;
}

vector<disks::file> disks::ls(){
    if(!_mounted_disk || !_mounted_partition){
        return {};
    }

    return fat32::ls(*_mounted_disk, *_mounted_partition, pwd);
}

uint64_t disks::free_size(){
    if(!_mounted_disk || !_mounted_partition){
        return 0;
    }

    return fat32::free_size(*_mounted_disk, *_mounted_partition);
}

vector<std::string>& disks::current_directory(){
    return pwd;
}

std::string disks::read_file(const std::string& file){
    if(!_mounted_disk || !_mounted_partition){
        return "";
    }

    return fat32::read_file(*_mounted_disk, *_mounted_partition, pwd, file);
}
