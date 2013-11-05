#include "disks.hpp"
#include "ata.hpp"
#include "thor.hpp"
#include "console.hpp"

namespace {

bool detected = false;

//For now, 4 is enough as only the ata driver is implemented
disks::disk_descriptor _disks[4];
uint64_t number_of_disks = 0;

void detect_disks(){
    ata::detect_disks();

    for(uint8_t i = 0; i < ata::number_of_disks(); ++i){
        auto& descriptor = ata::drive(i);

        if(descriptor.present){
            _disks[number_of_disks] = {number_of_disks, disks::disk_type::ATA, &descriptor};
            ++number_of_disks;
        }
    }

    detected = true;
}

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

} //end of anonymous namespace

uint64_t disks::detected_disks(){
    if(!detected){
        detect_disks();
    }

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

    //Unreachable
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

disks::partition_descriptor* disks::partitions(const disk_descriptor& disk){
    //TODO Can be leaked
    uint16_t* buffer = reinterpret_cast<uint16_t*>(k_malloc(512));

    if(!read_sectors(disk, 0, 1, buffer)){
        k_print_line("Read Boot Record failed");

        return nullptr;
    } else {
        auto* boot_record = reinterpret_cast<boot_record_t*>(buffer);

        if(boot_record->signature != 0xAA55){
            k_print_line("Invalid boot record signature");

            return nullptr;
        }

        uint64_t n = 0;
        for(int i = 0; i < 4; ++i){
            if(boot_record->partitions[i].type_code > 0){
                ++n;
            }
        }

        //TODO: This is a memory leak, partitions should be cached
        auto* partitions = reinterpret_cast<partition_descriptor*>(k_malloc(n * sizeof(partition_descriptor)));
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

        k_free(reinterpret_cast<uint64_t*>(buffer));

        return partitions;
    }
}
