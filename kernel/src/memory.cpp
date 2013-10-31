#include "types.hpp"
#include "memory.hpp"
#include "kernel_utils.hpp"

namespace {

struct mmap_entry {
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint16_t type;
    uint16_t acpi;
} __attribute__((packed));

std::size_t entry_count = 0;
mmap_entry e820_mmap[32];

} // end of anonymous namespace

void load_memory_map(){
    std::size_t failed = 0;
    __asm__ __volatile__ ("mov r8, 0" : : );
    interrupt<62>();
    __asm__ __volatile__ ("mov %0, rax" : : "a" (failed));

    if(!failed){
        std::size_t entry_count = 0;
        __asm__ __volatile__ ("mov r8, 1" : : );
        interrupt<62>();
        __asm__ __volatile__ ("mov %0, rax" : : "a" (entry_count));

        mmap_entry* address = 0;
        __asm__ __volatile__ ("mov r8, 2" : : );
        interrupt<62>();
        __asm__ __volatile__ ("mov %0, rax" : : "a" (address));
    }
}
