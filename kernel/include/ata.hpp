#ifndef ATA_H
#define ATA_H

#include "types.hpp"

struct drive_descriptor {
    uint16_t controller;
    uint8_t drive;
    bool present;
    uint8_t slave;
};

void detect_disks();
uint8_t number_of_disks();
drive_descriptor& drive(uint8_t disk);

bool ata_read_sectors(drive_descriptor& drive, std::size_t start, uint8_t count, void* destination);

#endif
