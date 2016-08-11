//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ATA_H
#define ATA_H

#include <types.hpp>
#include <string.hpp>

#include "fs/devfs.hpp"

namespace ata {

struct drive_descriptor {
    uint16_t controller;
    uint8_t drive;
    bool present;
    uint8_t slave;
    bool atapi;
    std::string model;
    std::string serial;
    std::string firmware;
};

void detect_disks();
uint8_t number_of_disks();
drive_descriptor& drive(uint8_t disk);

size_t read_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, void* destination, size_t& read);
size_t write_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, const void* source, size_t& written);

struct ata_driver : devfs::dev_driver {
    size_t read(void* data, char* buffer, size_t count, size_t offset, size_t& read);
    size_t write(void* data, const char* buffer, size_t count, size_t offset, size_t& written);
    size_t size(void* data);
};

struct ata_part_driver : devfs::dev_driver {
    size_t read(void* data, char* buffer, size_t count, size_t offset, size_t& read);
    size_t write(void* data, const char* buffer, size_t count, size_t offset, size_t& written);
    size_t size(void* data);
};

} // end of namespace ata

#endif
