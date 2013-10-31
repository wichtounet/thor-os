#ifndef MEMORY_H
#define MEMORY_H

#include "types.hpp"

struct mmapentry {
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint16_t type;
    uint16_t acpi;
    uint32_t damn_padding;
} __attribute__((packed));

void load_memory_map();
bool mmap_failed();
std::size_t mmap_entry_count();
const mmapentry& mmap_entry(std::size_t i);

#endif
