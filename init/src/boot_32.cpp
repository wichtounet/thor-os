//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#define CODE_32
#define THOR_INIT

#include <types.hpp>

#include "boot/boot_32.hpp"
#include "kernel.hpp"
#include "early_memory.hpp"
#include "virtual_debug.hpp"
#include "ata_constants.hpp"

static_assert(sizeof(uint8_t) == 1, "uint8_t must be 1 byte long");
static_assert(sizeof(uint16_t) == 2, "uint16_t must be 2 bytes long");
static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes long");
static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes long");

namespace {

constexpr const uint16_t COM1_PORT = 0x3f8;

//The size of page in memory
constexpr const size_t PAGE_SIZE = 4096;

void early_log(const char* s){
    virtual_debug(s);
    virtual_debug("\n");
    auto c = early::early_logs_count();
    auto table = reinterpret_cast<uint32_t*>(early::early_logs_address);
    table[c] = reinterpret_cast<uint32_t>(s);
    early::early_logs_count(c + 1);
}

void virtual_debug(const uint32_t& value){
    char buffer[32];

    if(value == 0){
        ::virtual_debug("0");
        return;
    }

    char int_buffer[20];
    int i = 0;
    auto rem = value;

    while(rem != 0){
        int_buffer[i++] = '0' + rem  % 10;
        rem /= 10;
    }

    --i;

    size_t j = 0;
    for(; i >= 0; --i){
        buffer[j++] = int_buffer[i];
    }

    buffer[j] = '\0';

    ::virtual_debug(buffer);
}

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

    early_log("PAE Activated");
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
    *reinterpret_cast<uint32_t*>(PML4T) = PML4T + PAGE_SIZE + 0x7;

    //PDPT[0] -> PDT
    *reinterpret_cast<uint32_t*>(PML4T + 1 * PAGE_SIZE) = PML4T + 2 * PAGE_SIZE + 0x7;

    //PD[0] -> PT
    *reinterpret_cast<uint32_t*>(PML4T + 2 * PAGE_SIZE) = PML4T + 3 * PAGE_SIZE + 0x7;

    //Map the first MiB

    auto page_table_ptr = reinterpret_cast<uint32_t*>(PML4T + 3 * PAGE_SIZE);
    auto phys = 0x3;
    for(uint32_t i = 0; i < 256; ++i){
        *page_table_ptr = phys;

        phys += PAGE_SIZE;

        //A page entry is 64 bit in size
        page_table_ptr += 2;
    }

    early_log("Paging configured");
}

void enable_long_mode(){
    asm volatile(
        "mov ecx, 0xC0000080 \t\n"
        "rdmsr \t\n"
        "or eax, 0b100000000 \t\n"
        "wrmsr \t\n");

    early_log("Long mode enabled");
}

void set_pml4t(){
    asm volatile(
        "mov eax, 0x70000 \t\n"  // Bass address of PML4
        "mov cr3, eax \t\n");    // load page-map level-4 base

    early_log("PML4T set");
}

void enable_paging(){
    asm volatile(
        "mov eax, cr0 \t\n"
        "or eax, 0b10000000000000000000000000000000 \t\n"
        "mov cr0, eax \t\n");

    early_log("Paging enabled");
}

void __attribute__((noreturn)) lm_jump(){
    //The trick done in boot_16 does not work, so just jump at the same
    //place and then call the function
    asm volatile("jmp 0x18:fake_label; fake_label:");

    //TODO kernel_main();

    __builtin_unreachable();
}

void serial_init() {
    out_byte(COM1_PORT + 1, 0x00);    // Disable all interrupts
    out_byte(COM1_PORT + 3, 0x80);    // Enable DLAB
    out_byte(COM1_PORT + 0, 0x03);    // 38400 baud
    out_byte(COM1_PORT + 1, 0x00);
    out_byte(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    out_byte(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    out_byte(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

inline void ata_400ns_delay(uint16_t controller){
    in_byte(controller + ATA_STATUS);
    in_byte(controller + ATA_STATUS);
    in_byte(controller + ATA_STATUS);
    in_byte(controller + ATA_STATUS);
    in_byte(controller + ATA_STATUS);
    in_byte(controller + ATA_STATUS);
    in_byte(controller + ATA_STATUS);
    in_byte(controller + ATA_STATUS);
    in_byte(controller + ATA_STATUS);
}

uint8_t wait_for_controller(uint16_t controller, uint8_t mask, uint8_t value, uint16_t timeout){
    uint8_t status;
    do {
        // Sleep at least 400ns before reading the status register
        ata_400ns_delay(controller);

        // Final read of the controller status
        status = in_byte(controller + ATA_STATUS);
    } while ((status & mask) != value && --timeout);

    return timeout;
}

bool ata_select_device(){
    auto wait_mask = ATA_STATUS_BSY | ATA_STATUS_DRQ;

    if(!wait_for_controller(ATA_PRIMARY, wait_mask, 0, 10000)){
        return false;
    }

    //Indicate the selected device
    out_byte(ATA_PRIMARY + ATA_DRV_HEAD, 0xA0 | (MASTER_BIT << 4));

    if(!wait_for_controller(ATA_PRIMARY, wait_mask, 0, 10000)){
        return false;
    }

    return true;
}

bool read_sector(uint64_t start, void* data){
    //Select the device
    if(!ata_select_device()){
        return false;
    }

    uint8_t sc = start & 0xFF;
    uint8_t cl = (start >> 8) & 0xFF;
    uint8_t ch = (start >> 16) & 0xFF;
    uint8_t hd = (start >> 24) & 0x0F;

    //Process the command
    out_byte(ATA_PRIMARY + ATA_NSECTOR, 1);
    out_byte(ATA_PRIMARY + ATA_SECTOR, sc);
    out_byte(ATA_PRIMARY + ATA_LCYL, cl);
    out_byte(ATA_PRIMARY + ATA_HCYL, ch);
    out_byte(ATA_PRIMARY + ATA_DRV_HEAD, (1 << 6) | (MASTER_BIT << 4) | hd);
    out_byte(ATA_PRIMARY + ATA_COMMAND, ATA_READ_BLOCK);

    //Wait at most 30 seconds for BSY flag to be cleared
    if(!wait_for_controller(ATA_PRIMARY, ATA_STATUS_BSY, 0, 30000)){
        return false;
    }

    //Verify if there are errors
    if(in_byte(ATA_PRIMARY + ATA_STATUS) & ATA_STATUS_ERR){
        return false;
    }

    // Polling

    while(true){
        ata_400ns_delay(ATA_PRIMARY);

        auto status = in_byte(ATA_PRIMARY + ATA_STATUS);

        if(status & ATA_STATUS_BSY){
            continue;
        }

        if(status & ATA_STATUS_ERR || status & ATA_STATUS_DF){
            return false;
        }

        if(status & ATA_STATUS_DRQ){
            break;
        }
    }

    //Read the disk sectors
    uint16_t* buffer = reinterpret_cast<uint16_t*>(data);
    for(int i = 0; i < 256; ++i){
        *buffer++ = in_word(ATA_PRIMARY + ATA_DATA);
    }

    return true;
}

void suspend_init(const char* message){
    early_log(message);
    asm volatile("cli; hlt");
    __builtin_unreachable();
}

void detect_disks(){
    // Disable all interrupts
    out_byte(ATA_PRIMARY + ATA_DEV_CTL, ATA_CTL_nIEN);
    out_byte(ATA_SECONDARY + ATA_DEV_CTL, ATA_CTL_nIEN);

    // Make sure the primary controller is enabled

    out_byte(ATA_PRIMARY + ATA_NSECTOR, 0xAB);
    if(in_byte(ATA_PRIMARY + ATA_NSECTOR) != 0xAB){
        suspend_init("The primary ATA controller is not enabled");
    }

    char block_buffer[512];

    // Read the MBR
    if(!read_sector(0, block_buffer)){
        suspend_init("Unable to read MBR");
    }

    // Extract the start of the partition
    uint16_t partition_start = *reinterpret_cast<uint16_t*>(block_buffer + 0x1be);

    // Reset interrupt status
    out_byte(ATA_PRIMARY + ATA_DEV_CTL, 0);
    out_byte(ATA_SECONDARY + ATA_DEV_CTL, 0);
}

} //end of anonymous namespace

bool serial_is_transmit_buffer_empty() {
   return in_byte(COM1_PORT + 5) & 0x20;
}

void serial_transmit(char a) {
   while (serial_is_transmit_buffer_empty() == 0){}

   out_byte(COM1_PORT, a);
}

extern "C" {

void pm_main(){
    //Update segments
    set_segments();

    //Activate PAE
    activate_pae();

    //Setup paging
    setup_paging();

    // Initialize serial transmission (for debugging in Qemu)
    serial_init();

    // TODO This will need to be computed from the init loader
    early::kernel_mib(1);

    //Enable long mode by setting the EFER.LME flag
    enable_long_mode();

    //Set the address of the PML4T
    set_pml4t();

    //Enable paging
    enable_paging();

    detect_disks();

    asm volatile("cli; hlt");
    __builtin_unreachable();

    //long mode jump
    lm_jump();
}

} //end of extern "C"
