#include "ata.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"
#include "memory.hpp"

namespace {

//IDE Controllers
#define ATA_PRIMARY 0x1F0
#define ATA_SECONDARY 0x170

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

//Master/Slave on devices
#define MASTER_BIT 0
#define SLAVE_BIT 1

bool detected = false;
drive_descriptor* drives;

volatile bool primary_invoked = false;
volatile bool secondary_invoked = false;

void primary_controller_handler(){
    primary_invoked = true;
}

void secondary_controller_handler(){
    secondary_invoked = true;
}

void ata_wait_irq_primary(){
    while(!primary_invoked){
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
    }

    primary_invoked = false;
}

void ata_wait_irq_secondary(){
    while(!secondary_invoked){
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
    }

    secondary_invoked = false;
}

} //end of anonymous namespace

void detect_disks(){
    if(!detected){
        drives = reinterpret_cast<drive_descriptor*>(k_malloc(4 * sizeof(drive_descriptor)));

        drives[0] = {ATA_PRIMARY, 0xE0, false, MASTER_BIT};
        drives[1] = {ATA_PRIMARY, 0xF0, false, SLAVE_BIT};
        drives[2] = {ATA_SECONDARY, 0xE0, false, MASTER_BIT};
        drives[3] = {ATA_SECONDARY, 0xF0, false, SLAVE_BIT};

        for(uint8_t i = 0; i < 4; ++i){
            auto& drive = drives[i];

            out_byte(drive.controller + 0x6, drive.drive);
            sleep_ms(4);
            drive.present = in_byte(drive.controller + 0x7) & 0x40;
        }

        register_irq_handler<14>(primary_controller_handler);
        register_irq_handler<15>(secondary_controller_handler);

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
    //Select the device
    if(!select_device(drive)){
        return false;
    }

    auto controller = drive.controller;

    uint8_t sc = start & 0xFF;
    uint8_t cl = (start >> 8) & 0xFF;
    uint8_t ch = (start >> 16) & 0xFF;
    uint8_t hd = (start >> 24) & 0x0F;

    //Process the command
    out_byte(controller + ATA_NSECTOR, count);
    out_byte(controller + ATA_SECTOR, sc);
    out_byte(controller + ATA_LCYL, cl);
    out_byte(controller + ATA_HCYL, ch);
    out_byte(controller + ATA_DRV_HEAD, (1 << 6) | (drive.slave << 4) | hd);
    out_byte(controller + ATA_COMMAND, ATA_READ_BLOCK);

    sleep_ms(1);

    //Wait at most 30 seconds for BSY flag to be cleared
    if(!wait_for_controller(controller, ATA_STATUS_BSY, 0, 30000)){
        return false;
    }

    //Verify if there are errors
    if(in_byte(controller + ATA_STATUS) & ATA_STATUS_ERR){
        return false;
    }

    //Wait the IRQ to happen
    if(controller == ATA_PRIMARY){
        ata_wait_irq_primary();
    } else {
        ata_wait_irq_secondary();
    }

    //The device can report an error after the IRQ
    if(in_byte(controller + ATA_STATUS) & ATA_STATUS_ERR){
        return false;
    }

    uint16_t* buffer = reinterpret_cast<uint16_t*>(destination);

    //Read the disk sectors
    for(uint8_t sector = 0; sector < count; ++sector){
        for(int i = 0; i < 256; ++i){
            *buffer++ = in_word(controller + ATA_DATA);
        }
    }

    return true;
}
