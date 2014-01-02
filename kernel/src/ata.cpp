//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ata.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"
#include "memory.hpp"
#include "thor.hpp"
#include "interrupts.hpp"
#include "console.hpp"

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

#define ATA_CTL_SRST    0x04
#define ATA_CTL_nIEN    0x02

//Master/Slave on devices
#define MASTER_BIT 0
#define SLAVE_BIT 1

ata::drive_descriptor* drives;

volatile bool primary_invoked = false;
volatile bool secondary_invoked = false;

//TODO In the future, the wait for IRQs, could
//be done with a semaphore

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

static uint8_t wait_for_controller(uint16_t controller, uint8_t mask, uint8_t value, uint16_t timeout){
    uint8_t status;
    do {
        status = in_byte(controller + ATA_STATUS);
        sleep_ms(1);
    } while ((status & mask) != value && --timeout);

    return timeout;
}

bool select_device(ata::drive_descriptor& drive){
    auto controller = drive.controller;

    auto wait_mask = ATA_STATUS_BSY | ATA_STATUS_DRQ;

    if(!wait_for_controller(controller, wait_mask, 0, 10000)){
        return false;
    }

    //Indicate the selected device
    out_byte(controller + ATA_DRV_HEAD, 0xA0 | (drive.slave << 4));

    //Sleep at least 400ns before reading the status register
    sleep_ms(1);

    if(!wait_for_controller(controller, wait_mask, 0, 10000)){
        return false;
    }

    return true;
}

bool read_write_sector(ata::drive_descriptor& drive, uint64_t start, void* data, bool read){
    //Select the device
    if(!select_device(drive)){
        return false;
    }

    auto controller = drive.controller;

    uint8_t sc = start & 0xFF;
    uint8_t cl = (start >> 8) & 0xFF;
    uint8_t ch = (start >> 16) & 0xFF;
    uint8_t hd = (start >> 24) & 0x0F;

    auto command = read ? ATA_READ_BLOCK : ATA_WRITE_BLOCK;

    //Process the command
    out_byte(controller + ATA_NSECTOR, 1);
    out_byte(controller + ATA_SECTOR, sc);
    out_byte(controller + ATA_LCYL, cl);
    out_byte(controller + ATA_HCYL, ch);
    out_byte(controller + ATA_DRV_HEAD, (1 << 6) | (drive.slave << 4) | hd);
    out_byte(controller + ATA_COMMAND, command);

    //Wait at least 400ns before reading status register
    sleep_ms(1);

    //Wait at most 30 seconds for BSY flag to be cleared
    if(!wait_for_controller(controller, ATA_STATUS_BSY, 0, 30000)){
        return false;
    }

    //Verify if there are errors
    if(in_byte(controller + ATA_STATUS) & ATA_STATUS_ERR){
        return false;
    }

    uint16_t* buffer = reinterpret_cast<uint16_t*>(data);

    if(!read){
        //Send the data to the controller
        for(int i = 0; i < 256; ++i){
            out_word(controller + ATA_DATA, *buffer++);
        }
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

    if(read){
        //Read the disk sectors
        for(int i = 0; i < 256; ++i){
            *buffer++ = in_word(controller + ATA_DATA);
        }
    }

    return true;
}

bool reset_controller(uint16_t controller){
    out_byte(controller + ATA_DEV_CTL, ATA_CTL_SRST);

    sleep_ms(5);

    //The controller should set the BSY flag after, SRST has been set
    if(!wait_for_controller(controller, ATA_STATUS_BSY, ATA_STATUS_BSY, 1000)){
        return false;
    }

    out_byte(controller + ATA_DEV_CTL, 0);

    //Wait at most 30 seconds for BSY flag to be cleared
    if(!wait_for_controller(controller, ATA_STATUS_BSY, 0, 30000)){
        return false;
    }

    return true;
}

void ide_string_into(std::string& destination, uint16_t* info, size_t start, size_t size){
    //Copy the characters
    auto t = reinterpret_cast<char*>(&info[start]);
    for(size_t i =0; i < size; ++i){
        destination += t[i];
    }

    //Swap characters
    for(size_t i = 0; i < size; i += 2){
        auto c = destination[i];
        destination[i] = destination[i + 1];
        destination[i+1] = c;
    }

    //Cleanup
    //TODO It is perhaps necessary to cleanup the data
}

void identify(ata::drive_descriptor& drive){
    drive.present = false;

    //First, test that the ATA controller of this drive is enabled
    out_byte(drive.controller + ATA_NSECTOR, 0xAB);
    if(in_byte(drive.controller + ATA_NSECTOR) != 0xAB){
        return;
    }

    //Reset the ATA controller
    reset_controller(drive.controller);

    //Try to select the drive
    if(!select_device(drive)){
        return;
    }

    //Once device has been selected sucessully, the drive is present
    drive.present = true;

    //Then try to obtain more data on the device
    out_byte(drive.controller + ATA_NSECTOR, 0);
    out_byte(drive.controller + ATA_SECTOR, 0);
    out_byte(drive.controller + ATA_LCYL, 0);
    out_byte(drive.controller + ATA_HCYL, 0);
    out_byte(drive.controller + ATA_COMMAND, ATA_IDENTIFY);
    sleep_ms(5);

    //Wait at most 30 seconds for BSY flag to be cleared
    if(!wait_for_controller(drive.controller, ATA_STATUS_BSY | ATA_STATUS_DRQ | ATA_STATUS_ERR, ATA_STATUS_DRQ, 30000)){
        //If the IDENTIFY fails, the disk is considered as present, but
        //we don't have any information about it
        return;
    }

    //Read the information

    uint16_t info[256];
    for(size_t i = 0; i < 256; ++i){
        info[i] = in_word(drive.controller + ATA_DATA);
    }

    //INFO: LBA and DMA feature can be tested here

    ide_string_into(drive.model, info, 27, 40);
    ide_string_into(drive.serial, info, 10, 20);
    ide_string_into(drive.firmware, info, 23, 8);
}

} //end of anonymous namespace

void ata::detect_disks(){
    drives = new drive_descriptor[4];

    drives[0] = {ATA_PRIMARY, 0xE0, false, MASTER_BIT, "", "", ""};
    drives[1] = {ATA_PRIMARY, 0xF0, false, SLAVE_BIT, "", "", ""};
    drives[2] = {ATA_SECONDARY, 0xE0, false, MASTER_BIT, "", "", ""};
    drives[3] = {ATA_SECONDARY, 0xF0, false, SLAVE_BIT, "", "", ""};

    for(uint8_t i = 0; i < 4; ++i){
        auto& drive = drives[i];

        identify(drive);
    }

    interrupt::register_irq_handler(14, primary_controller_handler);
    interrupt::register_irq_handler(15, secondary_controller_handler);
}

uint8_t ata::number_of_disks(){
    return 4;
}

ata::drive_descriptor& ata::drive(uint8_t disk){
    return drives[disk];
}

bool ata::read_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, void* destination){
    auto buffer = reinterpret_cast<uint8_t*>(destination);

    for(size_t i = 0; i < count; ++i){
        if(!read_write_sector(drive, start + i, buffer, true)){
            return false;
        }

        buffer += 512;
    }

    return true;
}

bool ata::write_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, void* source){
    auto buffer = reinterpret_cast<uint8_t*>(source);

    for(size_t i = 0; i < count; ++i){
        if(!read_write_sector(drive, start + i, buffer, false)){
            return false;
        }

        buffer += 512;
    }

    return true;
}
