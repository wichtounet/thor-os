//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#define CODE_32
#define THOR_INIT

#include <types.hpp>

#include "boot_32.hpp"
#include "kernel.hpp"
#include "early_memory.hpp"
#include "virtual_debug.hpp"
#include "drivers/ata_constants.hpp"

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

void virtual_debug(uint32_t value){
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

void virtual_debug_var(const char* message, uint32_t value){
    ::virtual_debug(message);
    virtual_debug(value);
    ::virtual_debug("\n");
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

void setup_kernel_paging(uint32_t kernel_mib){
    static_assert(early::kernel_address == 0x100000, "Only 0x100000 has been implemented");

    //Map all the kernel

    auto current_pt = 0;

    auto page_table_ptr = reinterpret_cast<uint32_t*>(PML4T + 3 * PAGE_SIZE + 256 * 8);
    auto phys = 0x100003;
    for(uint32_t i = 256; i < (1 + kernel_mib) * 256; ++i){
        *page_table_ptr = phys;

        phys += PAGE_SIZE;

        //A page entry is 64 bit in size
        page_table_ptr += 2;

        if(i % 511 == 0){
            ++current_pt;

            //PD[current_pt] -> PT
            *reinterpret_cast<uint32_t*>(PML4T + 2 * PAGE_SIZE + current_pt * 8) = PML4T + (3 + current_pt) * PAGE_SIZE + 0x7;
        }
    }
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

    reinterpret_cast<void (*)()>(early::kernel_address)();

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

bool read_cluster(uint64_t start, char* data, uint16_t sectors_per_cluster){
    for(uint16_t i = 0; i < sectors_per_cluster; ++i){
        if(!read_sector(start + i, data + i * 512)){
            return false;
        }
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
    uint16_t partition_start = *reinterpret_cast<uint16_t*>(block_buffer + 0x1be + 8);

    virtual_debug_var("partition_start: ", partition_start);

    // Read the VBR
    if(!read_sector(partition_start, block_buffer)){
        suspend_init("Unable to read VBR");
    }

    // Extract FAT informations
    auto sectors_per_cluster = *reinterpret_cast<uint8_t*>(block_buffer + 13);
    auto reserved_sectors = *reinterpret_cast<uint16_t*>(block_buffer + 14);
    auto number_of_fat = *reinterpret_cast<uint8_t*>(block_buffer + 16);
    auto sectors_per_fat = *reinterpret_cast<uint32_t*>(block_buffer + 36);
    auto root_dir_start = *reinterpret_cast<uint16_t*>(block_buffer + 44);

    uint16_t fat_begin = partition_start + reserved_sectors;
    uint32_t cluster_begin = number_of_fat * sectors_per_fat + fat_begin;
    uint16_t entries_per_sector = 512 / 32;
    uint16_t entries_per_cluster = entries_per_sector * sectors_per_cluster;

    virtual_debug_var("sectors_per_cluster: ", sectors_per_cluster);
    virtual_debug_var("reserved_sectors: ", reserved_sectors);
    virtual_debug_var("number_of_fat: ", number_of_fat);
    virtual_debug_var("sectors_per_fat: ", sectors_per_fat);
    virtual_debug_var("root_dir_start: ", root_dir_start);
    virtual_debug_var("fat_begin: ", fat_begin);
    virtual_debug_var("cluster_begin: ", cluster_begin);
    virtual_debug_var("entries_per_cluster: ", entries_per_cluster);

    auto root_lba = (root_dir_start - 2) * sectors_per_cluster + cluster_begin;

    if(!read_sector(root_lba, block_buffer)){
        suspend_init("Unable to read root directory");
    }

    uint16_t cluster_low = 0;
    uint16_t cluster_high = 0;
    uint32_t kernel_size = 0;

    for(size_t i = 0; i < entries_per_cluster; ++i){
        auto sector_index = i % entries_per_sector;

        if(*reinterpret_cast<uint8_t*>(block_buffer + sector_index * 32) == 0x0){
            suspend_init("Kernel executable not found");
        }

        if(*reinterpret_cast<uint32_t*>(block_buffer + sector_index * 32) == 0x4E52454B){ //NREK
            if(*reinterpret_cast<uint32_t*>(block_buffer + sector_index * 32 + 4) == 0x20204C45){ //spspLE
                if(*reinterpret_cast<uint32_t*>(block_buffer + sector_index * 32 + 6) == 0x49422020){ //IBspsp
                    if(*reinterpret_cast<uint8_t*>(block_buffer + sector_index * 32 + 10) == 0x4E){ //N
                        cluster_high = *reinterpret_cast<uint16_t*>(block_buffer + sector_index * 32 + 20);
                        cluster_low = *reinterpret_cast<uint16_t*>(block_buffer + sector_index * 32 + 26);
                        kernel_size = *reinterpret_cast<uint32_t*>(block_buffer + sector_index * 32 + 28);

                        break;
                    }
                }
            }
        }

        // Read the next sector
        if(i > 0 && i % entries_per_sector == 0){
            if(!read_sector(root_lba + i / entries_per_sector, block_buffer)){
                suspend_init("Unable to read root directory");
            }
        }
    }

    auto kernel_cluster = (uint32_t(cluster_high) << 16) + cluster_low;

    virtual_debug_var("kernel_cluster_first: ", kernel_cluster);
    virtual_debug_var("kernel_size: ", kernel_size);

    auto kernel_mib = kernel_size / 0x100000 + kernel_size % 0x100000 == 0 ? 0 : 1;
    early::kernel_mib(kernel_mib);

    setup_kernel_paging(kernel_mib);

    auto current_address = early::kernel_address;
    uint16_t clusters_read = 0;

    while(true){
        auto kernel_lba = (kernel_cluster - 2) * sectors_per_cluster + cluster_begin;
        if(!read_cluster(kernel_lba, reinterpret_cast<char*>(current_address), sectors_per_cluster)){
            suspend_init("Failed to read cluster from disk");
        }

        ++clusters_read;

        auto fat_sector = fat_begin + (kernel_cluster * 4) / 512;

        if(!read_sector(fat_sector, block_buffer)){
            suspend_init("Unable to read FAT sector");
        }

        auto* fat_table = reinterpret_cast<uint32_t*>(block_buffer);

        uint32_t next_cluster = fat_table[kernel_cluster % 128] & 0x0FFFFFFF;

        if(next_cluster == 0x0){
            suspend_init("Invalid FAT cluster: next cluster is free");
        }

        if(next_cluster == 0x1){
            suspend_init("Invalid FAT cluster: next cluster is reserved");
        }

        if(next_cluster == 0x0FFFFFF7){
            suspend_init("Invalid FAT cluster: next cluster is corrupted");
        }

        if(next_cluster == 0x0FFFFFF8 || next_cluster == 0x0FFFFFFF){
            break;
        }

        kernel_cluster = next_cluster;
        current_address += 512 * sectors_per_cluster;
    }

    virtual_debug_var("Number of clusters read for kernel: ", clusters_read);

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

    //Enable long mode by setting the EFER.LME flag
    enable_long_mode();

    //Set the address of the PML4T
    set_pml4t();

    //Enable paging
    enable_paging();

    detect_disks();

    //long mode jump
    lm_jump();
}

} //end of extern "C"
