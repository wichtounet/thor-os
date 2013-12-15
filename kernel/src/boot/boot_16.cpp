//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "boot/code16gcc.h"
#include "boot/boot_32.hpp"

namespace {

typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint64_t __attribute__ ((__mode__ (__DI__)));

static_assert(sizeof(uint8_t) == 1, "uint8_t must be 1 byte long");
static_assert(sizeof(uint16_t) == 2, "uint16_t must be 2 bytes long");
static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes long");
static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes long");

struct gdt_ptr {
    uint16_t length;
    uint32_t pointer;
} __attribute__ ((packed));

void out_byte(uint8_t value, uint16_t port){
    __asm__ __volatile__("out %1, %0" : : "a" (value), "dN" (port));
}

uint8_t in_byte(uint16_t port){
    uint8_t value;
    __asm__ __volatile__("in %0,%1" : "=a" (value) : "dN" (port));
    return value;
}

void set_ds(uint16_t seg){
    asm volatile("mov ds, %0" : : "rm" (seg));
}

void reset_segments(){
    set_ds(0);
}

void disable_interrupts(){
    __asm__ __volatile__ ("cli");
}

void enable_a20_gate(){
    //TODO This should really be improved:
    // 1. Test if a20 already enabled
    // 2- Use several methods of enabling if necessary until one succeeds

    //Enable A20 gate using fast method
    auto port_a = in_byte(0x92);
    port_a |=  0x02;
    port_a &= ~0x01;
    out_byte(port_a, 0x92);
}

void setup_idt(){
    static const gdt_ptr null_idt = {0, 0};
    asm volatile("lidt %0" : : "m" (null_idt));
}

constexpr uint64_t gdt_entry(uint32_t base, uint32_t limit, uint16_t flags){
    return  ((((base)  & 0xff000000ULL) << (56-24)) |
            (((flags) & 0x0000f0ffULL) << 40) |
            (((limit) & 0x000f0000ULL) << (48-16)) |
            (((base)  & 0x00ffffffULL) << 16) |
            (((limit) & 0x0000ffffULL)));
}

#define GDT_ENTRY(f1, f2)                       \
    ((static_cast<uint64_t>(0x0FFFF) << 48) |   \
     (((f1)) << 16) |                           \
     (((f2)) << 8))                             \

#define B_10011010 0x9A
#define B_11001111 0xCF
#define B_10010010 0x92
#define B_10001111 0x8F
#define B_10101111 0xAF

#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)

#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed

constexpr uint16_t code_32_selector(){
    return
        SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) |
        SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) |
        SEG_PRIV(0)     | SEG_CODE_EXRD;
}

constexpr uint16_t code_64_selector(){
    return
        SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) |
        SEG_LONG(1)     | SEG_SIZE(1) | SEG_GRAN(1) |
        SEG_PRIV(0)     | SEG_CODE_EXRD;
}

constexpr uint16_t data_selector(){
    return
        SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) |
        SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) |
        SEG_PRIV(0)     | SEG_DATA_RDWR;
}

void setup_gdt(){
    //TODO On some machines, this should be aligned to 16 bits
    static const uint64_t gdt[] = {
        0,                                      //Null Selector
        gdt_entry(0, 0xFFFFF, code_32_selector()),
        gdt_entry(0, 0xFFFFF, data_selector()),
        gdt_entry(0, 0xFFFFF, code_64_selector())

        //GDT_ENTRY(B_10011010, B_11001111),      //32-bit Code Selector (ring 0)
        //GDT_ENTRY(, ),      //Data Selector (ring 0)
        //GDT_ENTRY(, )       //64-bit Code Selector (ring 0)
    };

    static gdt_ptr gdtr;
    gdtr.length  = sizeof(gdt) - 1;
    gdtr.pointer = reinterpret_cast<uint32_t>(&gdt);

    asm volatile("lgdt [%0]" : : "m" (gdtr));
}

void protected_mode_enable(){
    asm volatile("mov eax, cr0; or al, 1; mov cr0, eax;");
}

void disable_paging(){
    asm volatile("mov eax, cr0; and eax, 0b01111111111111111111111111111111; mov cr0, eax;");
}

void __attribute__((noreturn)) pm_jump(){
    asm volatile(".byte 0x66, 0xea; .long pm_main; .word 0x8;");

    __builtin_unreachable();
}

} //end of anonymous namespace

void  __attribute__ ((noreturn)) rm_main(){
    //Make sure segments are clean
    reset_segments();

    //TODO Detect memory

    //Disable interrupts
    disable_interrupts();

    //Make sure a20 gate is enabled
    enable_a20_gate();

    //Setup an IDT with null limits to prevents interrupts from being used in
    //protected mode
    setup_idt();

    //Setup the GDT
    setup_gdt();

    //Switch to protected mode by activate PE bit of CR0
    protected_mode_enable();

    //Disable paging
    disable_paging();

    //Protected mode jump
    pm_jump();
}