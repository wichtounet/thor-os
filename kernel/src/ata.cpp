//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <lock_guard.hpp>

#include "ata.hpp"
#include "kernel_utils.hpp"
#include "timer.hpp"
#include "kalloc.hpp"
#include "thor.hpp"
#include "interrupts.hpp"
#include "console.hpp"
#include "errors.hpp"
#include "disks.hpp"
#include "mutex.hpp"

namespace {

static constexpr const size_t BLOCK_SIZE = 512;

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

mutex<> ata_lock;

mutex<> primary_lock;
mutex<> secondary_lock;

volatile bool primary_invoked = false;
volatile bool secondary_invoked = false;

//TODO In the future, the wait for IRQs, could
//be done with a semaphore

void primary_controller_handler(interrupt::syscall_regs*){
    if(scheduler::is_started()){
        primary_lock.release();
    } else {
        primary_invoked = true;
    }
}

void secondary_controller_handler(interrupt::syscall_regs*){
    if(scheduler::is_started()){
        secondary_lock.release();
    } else {
        secondary_invoked = true;
    }
}

void ata_wait_irq_primary(){
    if(scheduler::is_started()){
        primary_lock.acquire();
    } else {
        while(!primary_invoked){
            asm volatile ("nop");
            asm volatile ("nop");
            asm volatile ("nop");
            asm volatile ("nop");
            asm volatile ("nop");
        }

        primary_invoked = false;
    }
}

void ata_wait_irq_secondary(){
    if(scheduler::is_started()){
        secondary_lock.acquire();
    } else {
        while(!secondary_invoked){
            asm volatile ("nop");
            asm volatile ("nop");
            asm volatile ("nop");
            asm volatile ("nop");
            asm volatile ("nop");
        }

        secondary_invoked = false;
    }
}

inline void ata_400ns_delay(uint16_t /*controller*/){
    timer::sleep_ms(1);

    //TODO Ideally, we should have a real 400ns delay by simply reading the controller status 4 times
}

static uint8_t wait_for_controller(uint16_t controller, uint8_t mask, uint8_t value, uint16_t timeout){
    uint8_t status;
    do {
        // Sleep at least 400ns before reading the status register
        ata_400ns_delay(controller);

        // Final read of the controller status
        status = in_byte(controller + ATA_STATUS);
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

    if(!wait_for_controller(controller, wait_mask, 0, 10000)){
        return false;
    }

    return true;
}

bool read_write_sector(ata::drive_descriptor& drive, uint64_t start, void* data, bool read){
    std::lock_guard<decltype(ata_lock)> lock(ata_lock);

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

    timer::sleep_ms(5);

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
    char buffer[50];

    //Copy the characters
    auto t = reinterpret_cast<char*>(&info[start]);
    for(size_t i = 0; i < size; ++i){
        buffer[i] = t[i];
    }

    //Swap characters
    for(size_t i = 0; i < size; i += 2){
        auto c = buffer[i];
        buffer[i] = buffer[i + 1];
        buffer[i+1] = c;
    }

    //Cleanup the output
    size_t end = size - 1;
    while(true){
        auto c = buffer[end];

        if(c > 32 && c < 127){
            break;
        }

        if(end == 0){
            break;
        }

        --end;
    }

    buffer[end+1] = '\0';
    destination = buffer;
}

void identify(ata::drive_descriptor& drive){
    //First, test that the ATA controller of this drive is enabled
    //For that, test if data is resilient on the port
    out_byte(drive.controller + ATA_NSECTOR, 0xAB);
    if(in_byte(drive.controller + ATA_NSECTOR) != 0xAB){
        return;
    }

    //Select the device
    out_byte(drive.controller + ATA_DRV_HEAD, 0xA0 | (drive.slave << 4));
    timer::sleep_ms(1);

    //Generate the IDENTIFY command
    out_byte(drive.controller + ATA_COMMAND, ATA_IDENTIFY);
    timer::sleep_ms(1);

    //If status == 0, there are no device
    if(in_byte(drive.controller + ATA_STATUS) == 0){
        return;
    }

    //Verify if the device is ATA or not
    bool not_ata = false;
    while(1){
        auto status = in_byte(drive.controller + ATA_STATUS);

        if(status & ATA_STATUS_ERR){
            not_ata = true;
            break;
        }

        if(!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)){
            break;
        }
    }

    drive.atapi = false;

    //It is probably an ATAPI device
    if(not_ata){
        auto cl = in_byte(drive.controller + ATA_LCYL);
        auto ch = in_byte(drive.controller + ATA_HCYL);

        if(cl == 0x14 && ch == 0xEB){
            drive.atapi = true;
        } else if(cl == 0x69 && ch == 0x96){
            drive.atapi = true;
        } else {
            //Unknonw: ignoring
            return;
        }

        //Generate the ATAPI IDENTIFY command
        out_byte(drive.controller + ATA_COMMAND, 0xA1);
        timer::sleep_ms(1);
    }

    drive.present = true;

    // Read the informations
    uint16_t info[256];
    for(size_t b = 0; b < 256; ++b){
        info[b] = in_word(drive.controller + ATA_DATA);
    }

    //INFO: LBA and DMA feature can be tested here

    ide_string_into(drive.model, info, 27, 40);
    ide_string_into(drive.serial, info, 10, 20);
    ide_string_into(drive.firmware, info, 23, 8);
}

} //end of anonymous namespace

void ata::detect_disks(){
    ata_lock.init();

    primary_lock.init(0);
    secondary_lock.init(0);

    ata_lock.set_name("ata_lock");
    primary_lock.set_name("ata_primary_lock");
    secondary_lock.set_name("ata_secondary_lock");

    drives = new drive_descriptor[4];

    drives[0] = {ATA_PRIMARY, 0xE0, false, MASTER_BIT, false, "", "", ""};
    drives[1] = {ATA_PRIMARY, 0xF0, false, SLAVE_BIT, false, "", "", ""};
    drives[2] = {ATA_SECONDARY, 0xE0, false, MASTER_BIT, false, "", "", ""};
    drives[3] = {ATA_SECONDARY, 0xF0, false, SLAVE_BIT, false, "", "", ""};

    out_byte(ATA_PRIMARY + ATA_DEV_CTL, ATA_CTL_nIEN);
    out_byte(ATA_SECONDARY + ATA_DEV_CTL, ATA_CTL_nIEN);

    for(uint8_t i = 0; i < 4; ++i){
        auto& drive = drives[i];

        identify(drive);
    }

    out_byte(ATA_PRIMARY + ATA_DEV_CTL, 0);
    out_byte(ATA_SECONDARY + ATA_DEV_CTL, 0);

    interrupt::register_irq_handler(14, primary_controller_handler);
    interrupt::register_irq_handler(15, secondary_controller_handler);
}

uint8_t ata::number_of_disks(){
    return 4;
}

ata::drive_descriptor& ata::drive(uint8_t disk){
    return drives[disk];
}

size_t ata::ata_driver::read(void* data, char* destination, size_t count, size_t offset, size_t& read){
    if(count % BLOCK_SIZE != 0){
        return std::ERROR_INVALID_COUNT;
    }

    if(offset % BLOCK_SIZE != 0){
        return std::ERROR_INVALID_OFFSET;
    }

    read = 0;

    auto sectors = count / BLOCK_SIZE;
    auto start = offset / BLOCK_SIZE;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ata::drive_descriptor*>(descriptor->descriptor);

    return ata::read_sectors(*disk, start, sectors, destination, read);
}

size_t ata::ata_driver::write(void* data, const char* source, size_t count, size_t offset, size_t& written){
    if(count % BLOCK_SIZE != 0){
        return std::ERROR_INVALID_COUNT;
    }

    if(offset % BLOCK_SIZE != 0){
        return std::ERROR_INVALID_OFFSET;
    }

    written = 0;

    auto sectors = count / BLOCK_SIZE;
    auto start = offset / BLOCK_SIZE;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ata::drive_descriptor*>(descriptor->descriptor);

    return ata::write_sectors(*disk, start, sectors, source, written);
}

size_t ata::ata_part_driver::read(void* data, char* destination, size_t count, size_t offset, size_t& read){
    if(count % BLOCK_SIZE != 0){
        return std::ERROR_INVALID_COUNT;
    }

    if(offset % BLOCK_SIZE != 0){
        return std::ERROR_INVALID_OFFSET;
    }

    read = 0;

    auto sectors = count / BLOCK_SIZE;
    auto start = offset / BLOCK_SIZE;

    auto part_descriptor = reinterpret_cast<disks::partition_descriptor*>(data);
    auto descriptor = part_descriptor->disk;
    auto disk = reinterpret_cast<ata::drive_descriptor*>(descriptor->descriptor);

    start += part_descriptor->start;

    return ata::read_sectors(*disk, start, sectors, destination, read);
}

size_t ata::ata_part_driver::write(void* data, const char* source, size_t count, size_t offset, size_t& written){
    if(count % BLOCK_SIZE != 0){
        return std::ERROR_INVALID_COUNT;
    }

    if(offset % BLOCK_SIZE != 0){
        return std::ERROR_INVALID_OFFSET;
    }

    written = 0;

    auto sectors = count / BLOCK_SIZE;
    auto start = offset / BLOCK_SIZE;

    auto part_descriptor = reinterpret_cast<disks::partition_descriptor*>(data);
    auto descriptor = part_descriptor->disk;
    auto disk = reinterpret_cast<ata::drive_descriptor*>(descriptor->descriptor);

    start += part_descriptor->start;

    return ata::write_sectors(*disk, start, sectors, source, written);
}

size_t ata::read_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, void* destination, size_t& read){
    auto buffer = reinterpret_cast<uint8_t*>(destination);

    for(size_t i = 0; i < count; ++i){
        if(!read_write_sector(drive, start + i, buffer, true)){
            return std::ERROR_FAILED;
        }

        buffer += BLOCK_SIZE;
        read += BLOCK_SIZE;
    }

    return 0;
}

size_t ata::write_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, const void* source, size_t& written){
    auto buffer = reinterpret_cast<uint8_t*>(const_cast<void*>(source));

    for(size_t i = 0; i < count; ++i){
        if(!read_write_sector(drive, start + i, buffer, false)){
            return std::ERROR_FAILED;
        }

        buffer += BLOCK_SIZE;
        written += BLOCK_SIZE;
    }

    return 0;
}
