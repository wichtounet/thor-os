#include "ata.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"
#include "memory.hpp"

namespace {

bool detected = false;
drive_descriptor* drives;

} //end of anonymous namespace

#define MASTER_BIT 0
#define SLAVE_BIT 1

void detect_disks(){
    if(!detected){
        drives = reinterpret_cast<drive_descriptor*>(k_malloc(4 * sizeof(drive_descriptor)));

        drives[0] = {0x1F0, 0xE0, false, MASTER_BIT};
        drives[1] = {0x1F0, 0xF0, false, SLAVE_BIT};
        drives[2] = {0x170, 0xE0, false, MASTER_BIT};
        drives[3] = {0x170, 0xF0, false, SLAVE_BIT};

        for(uint8_t i = 0; i < 4; ++i){
            auto& drive = drives[i];

            out_byte(drive.controller + 0x6, drive.drive);
            sleep_ms(4);
            drive.present = in_byte(drive.controller + 0x7) & 0x40;
        }

        detected = true;
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

// I/O Controllers ports
#define ATA_DATA        0
#define ATA_ERROR       1
#define ATA_NSECTOR     2
#define ATA_SECTOR      3
#define ATA_LCYL        4
#define ATA_HCYL        5
#define ATA_DRV_HEAD    6
#define ATA_STATUS      7
#define ATA_COMMAND     7
#define ATA_DEV_CTL     0x206

// Status bits
#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01

// Commands
#define ATA_IDENTIFY    0xEC
#define ATAPI_IDENTIFY  0xA1
#define ATA_READ_BLOCK  0x20
#define ATA_WRITE_BLOCK 0x30

//TODO MOVE to anonymous

static uint8_t wait_for_controller(uint16_t controller, uint8_t mask, uint8_t value, uint16_t timeout){
    uint8_t status;
    do {
        status = in_byte(controller + ATA_STATUS);
        sleep_ms(1);
    } while ((status & mask) != value && --timeout);

    return timeout;
}

bool select_device(drive_descriptor& drive){
    auto controller = drive.controller;

    if(in_byte(controller + ATA_STATUS) & (ATA_STATUS_BSY | ATA_STATUS_DRQ)){
        return false;
    }

    out_byte(controller + ATA_DRV_HEAD, 0xA0 | (drive.slave << 4));
    sleep_ms(1);

    if(in_byte(controller + ATA_STATUS) & (ATA_STATUS_BSY | ATA_STATUS_DRQ)){
        return false;
    }

    return true;
}

bool ata_read_sectors(drive_descriptor& drive, std::size_t start, uint8_t count, void* destination){
    if(!select_device(drive)){
        return false;
    }

    auto controller = drive.controller;

    uint8_t sc = start & 0xFF;
    uint8_t cl = (start >> 8) & 0xFF;
    uint8_t ch = (start >> 16) & 0xFF;
    uint8_t hd = (start >> 24) & 0x0F;

    out_byte(controller + ATA_NSECTOR, count);
    out_byte(controller + ATA_SECTOR, sc);
    out_byte(controller + ATA_LCYL, cl);
    out_byte(controller + ATA_HCYL, ch);
    out_byte(controller + ATA_DRV_HEAD, (1 << 6) | (drive.slave << 4) | hd);
    out_byte(controller + ATA_COMMAND, ATA_READ_BLOCK);

    sleep_ms(1);

    if(!wait_for_controller(controller, ATA_STATUS_BSY, 0, 30000)){
        return false;
    }

    if(in_byte(controller + ATA_STATUS) & ATA_STATUS_ERR){
        return false;
    }

    uint16_t* buffer = reinterpret_cast<uint16_t*>(destination);

    for(uint8_t sector = 0; sector < count; ++sector){
        sleep_ms(1);

        while (!(in_byte(controller + ATA_STATUS) & ATA_STATUS_DRQ)) {
            __asm__ __volatile__ ("nop; nop;");
        }

        for(int i = 0; i < 256; ++i){
            *buffer++ = in_word(controller + ATA_DATA);
        }
    }

    return true;
}
