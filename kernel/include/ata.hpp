#ifndef ATA_H
#define ATA_H

#include "types.hpp"

struct drive_descriptor {
    uint16_t controller;
    uint8_t drive;
    bool present;
};

void detect_disks();
uint8_t number_of_disks();
drive_descriptor& drive(uint8_t disk);

void ata_read_sectors(drive_descriptor& drive, std::size_t start, uint8_t count, void* destination);

#endif
