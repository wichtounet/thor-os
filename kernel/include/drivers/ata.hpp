//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
    size_t size;
};

void detect_disks();
uint8_t number_of_disks();
drive_descriptor& drive(uint8_t disk);

size_t read_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, void* destination, size_t& read);
size_t write_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, const void* source, size_t& written);
size_t clear_sectors(drive_descriptor& drive, uint64_t start, uint8_t count, size_t& written);

struct ata_driver final : devfs::dev_driver {
    size_t read(void* data, char* buffer, size_t count, size_t offset, size_t& read) override;
    size_t write(void* data, const char* buffer, size_t count, size_t offset, size_t& written) override;
    size_t clear(void* data, size_t count, size_t offset, size_t& written) override;
    size_t size(void* data) override;
};

struct ata_part_driver final : devfs::dev_driver {
    size_t read(void* data, char* buffer, size_t count, size_t offset, size_t& read) override;
    size_t write(void* data, const char* buffer, size_t count, size_t offset, size_t& written) override;
    size_t clear(void* data, size_t count, size_t offset, size_t& written) override;
    size_t size(void* data) override;
};

} // end of namespace ata

#endif
