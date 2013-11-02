#include "ata.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"
#include "memory.hpp"

namespace {

bool detected = false;
drive_descriptor* drives;

} //end of anonymous namespace

void detect_disks(){
    if(!detected){
        drives = reinterpret_cast<drive_descriptor*>(k_malloc(4 * sizeof(drive_descriptor)));

        drives[0] = {0x1F0, 0xE0, false};
        drives[1] = {0x1F0, 0xF0, false};
        drives[2] = {0x170, 0xE0, false};
        drives[3] = {0x170, 0xF0, false};

        for(uint8_t i = 0; i < 4; ++i){
            auto& drive = drives[i];

            out_byte(drive.controller + 0x6, drive.drive);
            sleep_ms(4);
            drive.present = in_byte(drive.controller + 0x7) & 0x40;
        }
    }
}

uint8_t number_of_disks(){
    if(!detected){
        detect_disks();
    }

    return 4;
}

drive_descriptor& drive(uint8_t disk){
    if(!detected){
        detect_disks();
    }

    return drives[disk];
}

void ata_read_sectors(drive_descriptor& drive, std::size_t start, uint8_t count, void* destination){
    auto controller = drive.controller;

    out_byte(controller + 0x6, drive.drive | ((start >> 24) & 0x0F));  //TODO CHECK THIS

    out_byte(controller + 0x1, 0x00);
    out_byte(controller + 0x2, count);

    //Send the LBA28 value
    out_byte(controller + 0x3, static_cast<uint8_t>(start));
    out_byte(controller + 0x4, static_cast<uint8_t>(start >> 8));
    out_byte(controller + 0x5, static_cast<uint8_t>(start >> 16));
    //out_byte(controller + 0x6, drive | ((start >> 24) & 0x0F)); //TODO CHECK THIS

    //Send the read sectors command
    out_byte(controller + 0x7, 0x20);

    uint16_t* buffer = reinterpret_cast<uint16_t*>(destination);

    for(uint8_t sector = 0; sector < count; ++sector){
        sleep_ms(1);

        while (!(in_byte(0x1F7) & 0x08)) {
            __asm__ __volatile__ ("nop; nop;");
        }

        for(int i = 0; i < 256; ++i){
            (*buffer++) = in_word(controller + 0x0);
        }
    }
}
