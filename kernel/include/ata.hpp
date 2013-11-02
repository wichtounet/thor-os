#ifndef ATA_H
#define ATA_H

#include "types.hpp"

void ata_read_sectors(uint8_t controller, bool slave, std::size_t start, uint8_t count, void* destination);

#endif
