//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

/*
 * The implementation of the memory detection is made in boot_16.cpp
 * The finalization of the memory detection is made in e820.cpp once in long
 * mode.
 */

#ifndef E820_TYPES_HPP
#define E820_TYPES_HPP

#include <types.hpp>

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

struct mmapentry {
    uint64_t base;
    uint64_t size;
    uint64_t type;
};

} //end of namespace e820

#endif
