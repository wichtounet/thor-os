//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
 * The implementation of the memory detection is made in boot_16.cpp
 * The finalization of the memory detection is made in e820.cpp once in long
 * mode.
 */

#ifndef CODE_16
#include "stl/types.hpp"
#endif

#ifndef E820_HPP
#define E820_HPP

namespace e820 {

constexpr const uint32_t MAX_E820_ENTRIES = 20;

struct bios_e820_entry {
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint16_t type;
    uint16_t acpi;
} __attribute__((packed));

extern int16_t bios_e820_entry_count;
extern bios_e820_entry bios_e820_entries[MAX_E820_ENTRIES];

struct mmapentry {
    uint64_t base;
    uint64_t size;
    uint64_t type;
};

//Must be called by the kernel to transform e820 entries into mmap entries
void finalize_memory_detection();

bool mmap_failed();
uint64_t mmap_entry_count();
const mmapentry& mmap_entry(uint64_t i);
const char* str_e820_type(uint64_t type);

size_t available_memory();

} //end of namespace e820

#endif