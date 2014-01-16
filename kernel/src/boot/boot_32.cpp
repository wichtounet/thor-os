//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "boot/boot_32.hpp"
#include "kernel.hpp"

namespace {

typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint64_t __attribute__ ((__mode__ (__DI__)));

static_assert(sizeof(uint8_t) == 1, "uint8_t must be 1 byte long");
static_assert(sizeof(uint16_t) == 2, "uint16_t must be 2 bytes long");
static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes long");
static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes long");

const uint32_t PAGE_SIZE = 0x1000;
const uint32_t PML4T = 0x70000;

void set_segments(){
    asm volatile(
        "mov ax, 0x10; \t\n"
        "mov ds, ax \t\n"
        "mov es, ax \t\n"
        "mov fs, ax \t\n"
        "mov gs, ax \t\n"
        "mov ss, ax");
}

void activate_pae(){
    asm volatile("mov eax, cr4; or eax, 1 << 5; mov cr4, eax");
}

inline void clear_tables(uint32_t page){
    auto page_ptr = reinterpret_cast<uint32_t*>(page);

    for(uint32_t i = 0; i < (4 * 4096) / sizeof(uint32_t); ++i){
        *page_ptr++ = 0;
    }
}

void setup_paging(){
    //Clear all tables
    clear_tables(PML4T);

    //Link tables (0x3 means Writeable and Supervisor)

    //PML4T[0] -> PDPT
    *reinterpret_cast<uint32_t*>(PML4T) = PML4T + PAGE_SIZE + 0x3;

    //PDPT[0] -> PDT
    *reinterpret_cast<uint32_t*>(PML4T + 1 * PAGE_SIZE) = PML4T + 2 * PAGE_SIZE + 0x3;

    //PDT[0] -> PD
    *reinterpret_cast<uint32_t*>(PML4T + 2 * PAGE_SIZE) = PML4T + 3 * PAGE_SIZE + 0x3;

    //Map the first MiB

    auto page_table_ptr = reinterpret_cast<uint32_t*>(PML4T + 3 * PAGE_SIZE);
    auto phys = 0x3;
    for(uint32_t i = 0; i < 256; ++i){
        *page_table_ptr = phys;

        phys += 0x1000;

        //A page entry is 64 bit in size
        page_table_ptr += 2;
    }
}

void enable_long_mode(){
    asm volatile(
        "mov ecx, 0xC0000080 \t\n"
        "rdmsr \t\n"
        "or eax, 0b100000000 \t\n"
        "wrmsr \t\n");
}

void set_pml4t(){
    asm volatile(
        "mov eax, 0x70000 \t\n"  // Bass address of PML4
        "mov cr3, eax \t\n");    // load page-map level-4 base
}

void enable_paging(){
    asm volatile(
        "mov eax, cr0 \t\n"
        "or eax, 0b10000000000000000000000000000000 \t\n"
        "mov cr0, eax \t\n");
}

void __attribute__((noreturn)) lm_jump(){
    //The trick done in boot_16 does not work, so just jump at the same
    //place and then call the function
    asm volatile("jmp 0x18:fake_label; fake_label:");

    kernel_main();

    __builtin_unreachable();
}

} //end of anonymous namespace

extern "C" {

void pm_main(){
    //Update segments
    set_segments();

    //Activate PAE
    activate_pae();

    //Setup paging
    setup_paging();

    //Enable long mode by setting the EFER.LME flag
    enable_long_mode();

    //Set the address of the PML4T
    set_pml4t();

    //Enable paging
    enable_paging();

    //long mode jump
    lm_jump();
}

} //end of exern "C"
