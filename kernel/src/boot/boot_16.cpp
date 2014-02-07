//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#define CODE_16

#include "boot/code16gcc.h"
#include "boot/boot_32.hpp"

#include "gdt.hpp"
#include "e820.hpp" //Just for the address of the e820 map
#include "vesa.hpp"

//The Task State Segment
gdt::task_state_segment_t gdt::tss;

e820::bios_e820_entry e820::bios_e820_entries[e820::MAX_E820_ENTRIES];
int16_t e820::bios_e820_entry_count = 0;

vesa::vbe_info_block_t vesa::vbe_info_block;
vesa::mode_info_block_t vesa::mode_info_block;
bool vesa::vesa_enabled = false;

namespace {

constexpr const uint16_t DEFAULT_WIDTH = 1280;
constexpr const uint16_t DEFAULT_HEIGHT = 1024;
constexpr const uint16_t DEFAULT_BPP = 32;

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

void set_es(uint16_t seg){
    asm volatile("mov es, %0" : : "rm" (seg));
}

void set_gs(uint16_t seg){
    asm volatile("mov gs, %0" : : "rm" (seg));
}

void reset_segments(){
    set_ds(0);
    set_es(0);
}

int detect_memory_e820(){
    auto smap = &e820::bios_e820_entries[0];

    uint16_t entries = 0;

    uint32_t contID = 0;
    int signature;
    int bytes;

    static e820::bios_e820_entry buf;

    do {
        asm volatile ("int 0x15"
            : "=a"(signature), "=c"(bytes), "=b"(contID)
            : "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(&buf));

        if (signature != 0x534D4150){
            return -1;
        }

        if (bytes > 20 && (smap->acpi & 0x0001) == 0){
            // ignore this entry
        } else {
            *smap++ = buf;
            entries++;
        }
    } while (contID != 0 && entries < e820::MAX_E820_ENTRIES);

    return entries;
}

void detect_memory(){
    //TODO If e820 fails, try other solutions to get memory map

    e820::bios_e820_entry_count = detect_memory_e820();
}

uint16_t read_mode(uint16_t i){
    uint16_t mode;
    asm volatile("mov gs, %2; mov si, %1; add si, %3; mov %0, gs:[si]"
        : "=a" (mode)
        : "rm" (vesa::vbe_info_block.video_modes_ptr[0]),
          "rm" (vesa::vbe_info_block.video_modes_ptr[1]),
          "rm" (i));
    return mode;
}

uint16_t abs_diff(uint16_t a, uint16_t b){
    if(a > b){
        return a - b;
    } else {
        return b - a;
    }
}

template<typename T>
constexpr bool bit_set(const T& value, uint8_t bit){
    return value & (1 << bit);
}

void setup_vesa(){
    vesa::vbe_info_block.signature[0] = 'V';
    vesa::vbe_info_block.signature[1] = 'B';
    vesa::vbe_info_block.signature[2] = 'E';
    vesa::vbe_info_block.signature[3] = '2';

    return;

    uint16_t return_code;
    asm volatile ("int 0x10"
        : "=a"(return_code)
        : "a"(0x4F00), "D"(&vesa::vbe_info_block)
        : "memory");

    if(return_code == 0x4F){
        uint16_t best_mode = 0;
        uint16_t best_size_diff = 65535;

        bool one = false;

        for(uint16_t i = 0, mode = read_mode(i); mode != 0xFFFF; mode = read_mode(2 * ++i)){
            asm volatile ("int 0x10"
                : "=a"(return_code)
                : "a"(0x4F01), "c"(mode), "D"(&vesa::mode_info_block)
                : "memory");

            //Make sure the mode is supported by get mode info function
            if(return_code != 0x4F){
                continue;
            }

            //Check that the mode support Linear Frame Buffer
            if(!bit_set(vesa::mode_info_block.mode_attributes, 7)){
                continue;
            }

            //Make sure it is a packed pixel or direct color model
            if(vesa::mode_info_block.memory_model != 4 && vesa::mode_info_block.memory_model != 6){
                continue;
            }

            if(vesa::mode_info_block.bpp != DEFAULT_BPP){
                continue;
            }

            one = true;

            auto x_res = vesa::mode_info_block.width;
            auto y_res = vesa::mode_info_block.height;

            auto size_diff = abs_diff(x_res, DEFAULT_WIDTH) + abs_diff(y_res, DEFAULT_HEIGHT);

            if(size_diff < best_size_diff){
                best_mode = mode;
                best_size_diff = size_diff;
            }
        }

        if(!one || best_mode == 0xFFFF){
            vesa::vesa_enabled = false;
        } else {
            best_mode = best_mode | 0x4000;

            asm volatile ("int 0x10"
                : "=a"(return_code)
                : "a"(0x4F01), "c"(best_mode), "D"(&vesa::mode_info_block)
                : "memory");

            if(return_code == 0x4F){
                asm volatile ("int 0x10"
                    : "=a"(return_code)
                    : "a"(0x4F02), "b"(best_mode));

                vesa::vesa_enabled = return_code == 0x4F;
            } else {
                vesa::vesa_enabled = false;
            }
        }
    }

    set_gs(0);
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
    static const gdt::gdt_ptr null_idt = {0, 0};
    asm volatile("lidt %0" : : "m" (null_idt));
}

gdt::gdt_descriptor_t null_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    //zero-out the descriptor;
    *(reinterpret_cast<uint64_t*>(&descriptor)) = 0;

    return descriptor;
}

gdt::gdt_descriptor_t code_32_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_CODE_EXRD;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 0;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 1;
    descriptor.long_mode = 0;
    descriptor.granularity = 1;

    return descriptor;
}

gdt::gdt_descriptor_t code_64_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_CODE_EXRD;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 0;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 0;
    descriptor.long_mode = 1;
    descriptor.granularity = 1;

    return descriptor;
}

gdt::gdt_descriptor_t user_code_64_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_CODE_EXRD;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 3;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 0;
    descriptor.long_mode = 1;
    descriptor.granularity = 1;

    return descriptor;
}

gdt::gdt_descriptor_t data_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_DATA_RDWR;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 0;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 1;
    descriptor.long_mode = 0;
    descriptor.granularity = 1;

    return descriptor;
}

gdt::gdt_descriptor_t user_data_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_DATA_RDWR;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 3;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 0;
    descriptor.long_mode = 1;
    descriptor.granularity = 1;

    return descriptor;
}

//TODO On some machines, this should be aligned to 16 bits
static gdt::gdt_descriptor_t gdt[8];

static gdt::gdt_ptr gdtr;

void setup_gdt(){
    //1. Init GDT descriptor
    gdt[0] = null_descriptor();
    gdt[1] = code_32_descriptor();
    gdt[2] = data_descriptor();
    gdt[3] = code_64_descriptor();
    gdt[4] = user_code_64_descriptor();
    gdt[5] = user_data_descriptor();

    //2. Init TSS Descriptor

    uint32_t base = reinterpret_cast<uint32_t>(&gdt::tss);
    uint32_t limit = base + sizeof(gdt::task_state_segment_t);

    auto tss_selector = reinterpret_cast<gdt::tss_descriptor_t*>(&gdt[6]);
    tss_selector->type = gdt::SEG_TSS_AVAILABLE;
    tss_selector->always_0_1 = 0;
    tss_selector->always_0_2 = 0;
    tss_selector->always_0_3 = 0;
    tss_selector->dpl = 3;
    tss_selector->present = 1;
    tss_selector->avl = 0;
    tss_selector->granularity = 0;

    tss_selector->base_low = base & 0xFFFFFF;              //Bottom 24 bits
    tss_selector->base_middle = (base & 0xFF000000) >> 24; //Top 8 bits
    tss_selector->base_high = 0;                           //Top 32 bits are clear

    tss_selector->limit_low = limit & 0xFFFF;              //Low 16 bits
    tss_selector->limit_high = (limit & 0xF0000) >> 16;      //Top 4 bits

    //3. Init the GDT Pointer

    gdtr.length  = sizeof(gdt) - 1;
    gdtr.pointer = reinterpret_cast<uint32_t>(&gdt);

    //4. Load the GDT

    asm volatile("lgdt [%0]" : : "m" (gdtr));

    //5. Zero-out the TSS
    auto tss_ptr = reinterpret_cast<char*>(&gdt::tss);
    for(unsigned int i = 0; i < sizeof(gdt::task_state_segment_t); ++i){
        *tss_ptr++ = 0;
    }
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

    //Analyze memory
    detect_memory();

    //Make sure a20 gate is enabled
    enable_a20_gate();

    //Enable VESA
    setup_vesa();

    //Disable interrupts
    disable_interrupts();

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