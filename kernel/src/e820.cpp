//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "e820.hpp"

namespace {

e820::mmapentry e820_mmap[e820::MAX_E820_ENTRIES];
size_t _available_memory;

} //end of namespace anonymous

void e820::finalize_memory_detection(){
    auto t = *reinterpret_cast<int16_t*>(0x5000);
    auto smap = reinterpret_cast<e820::bios_e820_entry*>(0x5008);
    if(t > 0){
        for(int64_t i = 0; i < t; ++i){
            auto& bios_entry = smap[i];
            auto& os_entry = e820_mmap[i];

            uint64_t base = bios_entry.base_low + (static_cast<uint64_t>(bios_entry.base_high) << 32);
            uint64_t length = bios_entry.length_low + (static_cast<uint64_t>(bios_entry.length_high) << 32);

            os_entry.base = base;
            os_entry.size = length;
            os_entry.type = bios_entry.type;

            if(os_entry.base == 0 && os_entry.type == 1){
                os_entry.type = 7;
            }

            if(os_entry.type == 1){
                _available_memory += os_entry.size;
            }
        }
    }
}

uint64_t e820::mmap_entry_count(){
    auto t = *reinterpret_cast<int16_t*>(0x5000);
    return t;
    //return bios_e820_entry_count;
}

bool e820::mmap_failed(){
    auto t = *reinterpret_cast<int16_t*>(0x5000);
    return t <= 0;
}

const e820::mmapentry& e820::mmap_entry(uint64_t i){
    return e820_mmap[i];
}

const char* e820::str_e820_type(uint64_t type){
    switch(type){
        case 1:
            return "Free";
        case 2:
            return "Reserved";
        case 3:
        case 4:
            return "ACPI";
        case 5:
            return "Unusable";
        case 6:
            return "Disabled";
        case 7:
            return "Kernel";
        default:
            return "Unknown";
    }
}

size_t e820::available_memory(){
    return _available_memory;
}
