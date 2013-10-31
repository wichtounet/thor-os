#include "memory.hpp"

namespace {

struct bios_mmap_entry {
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint16_t type;
    uint16_t acpi;
    uint32_t damn_padding;
} __attribute__((packed));

std::size_t e820_failed = 0;
std::size_t entry_count = 0;
bios_mmap_entry* e820_address = 0;

mmapentry e820_mmap[32];

void mmap_query(std::size_t cmd, std::size_t* result){
    std::size_t tmp;
    __asm__ __volatile__ ("mov r8, %0; int 62; mov %1, rax" : : "dN" (cmd), "a" (tmp));
    *result = tmp;
}

}

void load_memory_map(){
    mmap_query(0, &e820_failed);
    mmap_query(1, &entry_count);
    mmap_query(2, (std::size_t*) &e820_address);

    if(!e820_failed && e820_address){
        for(std::size_t i = 0; i < entry_count; ++i){
            auto& bios_entry = e820_address[i];
            auto& os_entry = e820_mmap[i];

            std::size_t base = bios_entry.base_low + ((std::size_t) bios_entry.base_high << 32);
            std::size_t length = bios_entry.length_low + ((std::size_t) bios_entry.length_high << 32);

            os_entry.base = base;
            os_entry.size = length;
            os_entry.type = bios_entry.type;
        }
    }
}

std::size_t mmap_entry_count(){
    return entry_count;
}

bool mmap_failed(){
    return e820_failed;
}

const mmapentry& mmap_entry(std::size_t i){
    return e820_mmap[i];
}
