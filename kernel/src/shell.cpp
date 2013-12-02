//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "types.hpp"
#include "keyboard.hpp"
#include "kernel_utils.hpp"
#include "console.hpp"
#include "shell.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "memory.hpp"
#include "disks.hpp"
#include "string.hpp"
#include "vector.hpp"
#include "algorithms.hpp"

namespace {

vector<char*> history;
uint64_t history_index;

bool shift = false;

//Declarations of the different functions

void reboot_command(const vector<string>& params);
void help_command(const vector<string>& params);
void uptime_command(const vector<string>& params);
void clear_command(const vector<string>& params);
void date_command(const vector<string>& params);
void sleep_command(const vector<string>& params);
void echo_command(const vector<string>& params);
void mmap_command(const vector<string>& params);
void memory_command(const vector<string>& params);
void disks_command(const vector<string>& params);
void partitions_command(const vector<string>& params);
void mount_command(const vector<string>& params);
void unmount_command(const vector<string>& params);
void ls_command(const vector<string>& params);
void cd_command(const vector<string>& params);
void pwd_command(const vector<string>& params);
void free_command(const vector<string>& params);
void sysinfo_command(const vector<string>& params);

struct command_definition {
    const char* name;
    void (*function)(const vector<string>&);
};

command_definition commands[18] = {
    {"reboot", reboot_command},
    {"help", help_command},
    {"uptime", uptime_command},
    {"clear", clear_command},
    {"date", date_command},
    {"sleep", sleep_command},
    {"echo", echo_command},
    {"mmap", mmap_command},
    {"memory", memory_command},
    {"disks", disks_command},
    {"partitions", partitions_command},
    {"mount", mount_command},
    {"unmount", unmount_command},
    {"ls", ls_command},
    {"free", free_command},
    {"cd", cd_command},
    {"pwd", pwd_command},
    {"sysinfo", sysinfo_command},
};

uint64_t current_input_length = 0;
char current_input[50];

void exec_command();

void start_shell(){
    while(true){
        auto key = keyboard::get_char();

        //Key released
        if(key & 0x80){
            key &= ~(0x80);
            if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                shift = false;
            }
        }
        //Key pressed
        else {
            //ENTER validate the command
            if(key == keyboard::KEY_ENTER){
                current_input[current_input_length] = '\0';

                k_print_line();

                if(current_input_length > 0){
                    exec_command();

                    if(get_column() != 0){
                        k_print_line();
                    }

                    current_input_length = 0;
                }

                k_print("thor> ");
            } else if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                shift = true;
            } else if(key == keyboard::KEY_UP || key == keyboard::KEY_DOWN){
                if(history.size() > 0){
                    if(key == keyboard::KEY_UP){
                        if(history_index == 0){
                            continue;
                        }

                        --history_index;
                    } else { //KEY_DOWN
                        if(history_index == history.size()){
                            continue;
                        }

                        ++history_index;
                    }

                    set_column(6);

                    for(uint64_t i = 0; i < current_input_length; ++i){
                        k_print(' ');
                    }

                    set_column(6);

                    current_input_length = 0;

                    if(history_index < history.size()){
                        auto saved = history[history_index];
                        while(*saved){
                            current_input[current_input_length++] = *saved;
                            k_print(*saved);

                            ++saved;
                        }
                    }
                }
            } else if(key == keyboard::KEY_BACKSPACE){
                if(current_input_length > 0){
                    k_print('\b');

                    --current_input_length;
                }
            } else {
                auto qwertz_key =
                    shift
                    ? keyboard::shift_key_to_ascii(key)
                    : keyboard::key_to_ascii(key);

                if(qwertz_key){
                    current_input[current_input_length++] = qwertz_key;

                    k_print(qwertz_key);
                }
            }
        }
    }
}

void exec_command(){
    char buffer[50];

    auto saved = new char[current_input_length + 1];
    memcopy(saved, current_input, current_input_length);
    saved[current_input_length] = '\0';

    history.push_back(saved);
    history_index = history.size();

    for(auto& command : commands){
        const char* input_command = current_input;
        if(str_contains(current_input, ' ')){
            str_copy(current_input, buffer);
            input_command = str_until(buffer, ' ');
        }

        if(str_equals(input_command, command.name)){
            auto params = split(string(current_input));;

            command.function(params);

            return;
        }
    }

    k_printf("The command \"%s\" does not exist\n", current_input);
}

void clear_command(const vector<string>&){
    wipeout();
}

void reboot_command(const vector<string>&){
    interrupt<60>();
}

void help_command(const vector<string>&){
    k_print("Available commands:\n");

    for(auto& command : commands){
        k_print('\t');
        k_print_line(command.name);
    }
}

void uptime_command(const vector<string>&){
    k_printf("Uptime: %ds\n", timer_seconds());
}

#define CURRENT_YEAR        2013
#define cmos_address        0x70
#define cmos_data           0x71

int get_update_in_progress_flag() {
    out_byte(cmos_address, 0x0A);
    return (in_byte(cmos_data) & 0x80);
}

uint8_t get_RTC_register(int reg) {
    out_byte(cmos_address, reg);
    return in_byte(cmos_data);
}

void date_command(const vector<string>&){
    uint64_t second;
    uint64_t minute;
    uint64_t hour;
    uint64_t day;
    uint64_t month;
    uint64_t year;

    uint64_t last_second;
    uint64_t last_minute;
    uint64_t last_hour;
    uint64_t last_day;
    uint64_t last_month;
    uint64_t last_year;
    uint64_t registerB;

    //TODO When ACPI gets supported, get the address
    //of the century register and use it to make
    //better year calculation

    while (get_update_in_progress_flag()){};                // Make sure an update isn't in progress

    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);

    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;

        while (get_update_in_progress_flag()){};           // Make sure an update isn't in progress

        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);
    } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
        (last_day != day) || (last_month != month) || (last_year != year) );

    registerB = get_RTC_register(0x0B);

    // Convert BCD to binary values if necessary

    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);

    }

    // Convert 12 hour clock to 24 hour clock if necessary

    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year

    year += (CURRENT_YEAR / 100) * 100;
    if(year < CURRENT_YEAR){
        year += 100;
    }

    k_printf("%d.%d.%d %d:%.2d:%.2d\n", day, month, year, hour, minute, second);
}

void sleep_command(const vector<string>& params){
    sleep_ms(parse(params[1]) * 1000);
}

void echo_command(const vector<string>& params){
    for(uint64_t i = 1; i < params.size(); ++i){
        k_print(params[i]);
        k_print(' ');
    }
    k_print_line();
}

void mmap_command(const vector<string>&){
    if(mmap_failed()){
        k_print_line("The mmap was not correctly loaded from e820");
    } else {
        k_printf("There are %d mmap entry\n", mmap_entry_count());

        k_print_line("Base         End          Size                  Type");
        for(uint64_t i = 0; i < mmap_entry_count(); ++i){
            auto& entry = mmap_entry(i);

            k_printf("%.10h %.10h %.10h %8m %s\n",
                entry.base, entry.base + entry.size, entry.size, entry.size, str_e820_type(entry.type));
        }
    }
}

void memory_command(const vector<string>&){
    if(mmap_failed()){
        k_print_line("The mmap was not correctly loaded from e820");
    } else {
        k_printf("Total available memory: %m\n", available_memory());
        k_printf("Total used memory: %m\n", used_memory());
        k_printf("Total free memory: %m\n", free_memory());
        k_printf("Total allocated memory: %m\n", allocated_memory());
    }
}

void disks_command(const vector<string>&){
    k_print_line("UUID       Type");

    for(uint64_t i = 0; i < disks::detected_disks(); ++i){
        auto& descriptor = disks::disk_by_index(i);

        k_printf("%10d %s\n", descriptor.uuid, disks::disk_type_to_string(descriptor.type));
    }
}

void partitions_command(const vector<string>& params){
    auto uuid = parse(params[1]);

    if(disks::disk_exists(uuid)){
        auto partitions = disks::partitions(disks::disk_by_uuid(uuid));

        if(partitions.size() > 0){
            k_print_line("UUID       Type         Start      Sectors");

            for(auto& partition : partitions){
                k_printf("%10d %12s %10d %d\n", partition.uuid,
                    disks::partition_type_to_string(partition.type),
                    partition.start, partition.sectors);
            }
        }
    } else {
        k_printf("Disks %d does not exist\n", uuid);
    }
}

void mount_command(const vector<string>& params){
    if(params.size() == 1){
        auto md = disks::mounted_disk();
        auto mp = disks::mounted_partition();

        if(md && mp){
            k_printf("%d:%d is mounted\n", md->uuid, mp->uuid);
        } else {
            k_print_line("Nothing is mounted");
        }
    } else {
        auto disk_uuid = parse(params[1]);
        auto partition_uuid = parse(params[2]);

        if(disks::disk_exists(disk_uuid)){
            auto& disk = disks::disk_by_uuid(disk_uuid);
            if(disks::partition_exists(disk, partition_uuid)){
                disks::mount(disk, partition_uuid);
            } else {
                k_printf("Partition %d does not exist\n", partition_uuid);
            }
        } else {
            k_printf("Disk %d does not exist\n", disk_uuid);
        }
    }
}

void unmount_command(const vector<string>& ){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        k_print_line("Nothing is mounted");

        return;
    }

    disks::unmount();
}

void ls_command(const vector<string>& params){
    //By default hidden files are not shown
    bool show_hidden_files = false;

    //Read options if any
    if(params.size() > 1){
        for(size_t i = 1; i < params.size(); ++i){
            /*if(params[i] == "-a"){
                show_hidden_files = true;
            }*/
        }
    }

    if(!disks::mounted_partition() || !disks::mounted_disk()){
        k_print_line("Nothing is mounted");

        return;
    }

    auto files = disks::ls();

    for(auto& file : files){
        if(file.hidden && !show_hidden_files){
            continue;
        }

        k_print(file.name, 11);

        if(file.directory){
            k_print(" directory ");
        } else {
            k_print(" file ");
        }

        if(file.hidden){
            k_print(" hidden ");
        }

        if(file.system){
            k_print(" os ");
        }

        k_print_line(file.size);
    }
}

void free_command(const vector<string>&){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        k_print_line("Nothing is mounted");

        return;
    }

    k_printf("Free size: %m\n", disks::free_size());
}

void pwd_command(const vector<string>&){
    auto cd = disks::current_directory();

    if(cd.empty()){
        k_print_line("/");
    } else {
        k_printf("/%s\n", cd.c_str());
    }
}

void cd_command(const vector<string>& params){
    if(params.size() == 1){
        disks::set_current_directory();
    } else {
        disks::set_current_directory(params[1]);
    }
}

void native_cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx){
    /* ecx is often an input as well as an output. */
    asm volatile("cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "0" (*eax), "2" (*ecx));
}

// EDX Features
const int FPU = 1 << 0;
const int MMX = 1 << 23;
const int SSE = 1 << 25;
const int SSE2 = 1 << 26;
const int HT = 1 << 28;

//EAX Features
const int SSE3 = 1 << 9;
const int SSE41 = 1 << 19;
const int SSE42 = 1 << 20;
const int AES = 1 << 25;
const int AVX = 1 << 28;

void test_feature(uint32_t reg, int mask, const char* s){
    if(reg & mask){
        k_print(' ');
        k_print(s);
    }
}


void decode_bytes (int data, int descriptor[16], int *next){
    int i;

    i = *next;
    if (!(data & 0x80000000)) {
        descriptor[i++] =  data & 0x000000FF;
        descriptor[i++] = (data & 0x0000FF00) /      256; // 1 bytes R
        descriptor[i++] = (data & 0x00FF0000) /    65536; // 2 bytes R
        descriptor[i++] = (data & 0xFF000000) / 16777216; // 3 bytes R
        *next = i;
    }
}

void get_cache_info() {
    int next = 0, i = 0;
    int descriptor[256];
    int mem_count;

    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    k_print_line("Cache and TLB:");

    eax = 0;
    native_cpuid(&eax, &ebx, &ecx, &edx);

    if (eax < 2){
        k_print_line("   CPUID(2) not supported");
        return;
    }

    eax = 2;
    native_cpuid(&eax, &ebx, &ecx, &edx);

    mem_count = eax & 0x000000FF;        // 1st byte is the count
    eax &= 0xFFFFFF00;                   // mask off the count

    int* desc = descriptor;

    while ( i < mem_count) {
        decode_bytes(eax, desc, &next);
        decode_bytes(ebx, desc, &next);
        decode_bytes(ecx, desc, &next);
        decode_bytes(edx, desc, &next);

        ++i;

        eax = 2;
        ecx = i;
        native_cpuid(&eax, &ebx, &ecx, &edx);
        desc += 16;
    }

    for (i=0; i< next; i++)     {
        if ( descriptor[i] ==  0x00){
            // NULL descriptor, legal value but no info
            continue;
        }

        if ( descriptor[i] ==  0x01)
            k_print_line("  Instruction TLB ...   4 kb pages, 4-way associative, 32 entries");
        if ( descriptor[i] ==  0x02)
            k_print_line("  Instruction TLB ...   4 Mb pages, 4-way associative, 2 entries");
        if ( descriptor[i] ==  0x03)
            k_print_line("  Data TLB ..........   4 kb pages, 4-way associative, 64 entries");
        if ( descriptor[i] ==  0x04)
            k_print_line("  Data TLB ..........   4 Mb pages, 4-way associative, 8 entries");
        if ( descriptor[i] ==  0x06)
            k_print_line("  L1 instruction cache  8 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x08)
            k_print_line("  L1 instruction cache 16 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x0A)
            k_print_line("  L1 data cache .....   8 kb, 2-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x0B)
            k_print_line("  Instruction TLB ...   4 Mb pages, 4-way associative, 4 entries");
        if ( descriptor[i] ==  0x0C)
            k_print_line("  L1 data cache .....  16 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x22){
            k_print_line("  L3 cache:     512K Bytes, 4-way associative, 64 byte line size, ");
            k_print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x23){
            k_print_line("  L3 cache:     1M Bytes, 8-way associative, 64 byte line size, ");
            k_print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x25){
            k_print_line("  L3 cache:     2M Bytes, 8-way associative, 64 byte line size, ");
            k_print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x29){
            k_print_line("  L3 cache:     4M Bytes, 8-way associative, 64 byte line size, ");
            k_print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x2C)
            k_print_line("  1st-level D-cache:   32K Bytes, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x30)
            k_print_line("  1st-level I-cache:   32K Bytes, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x40)
            k_print_line("  No L2 cache OR if there is an L2 cache, then no L3 cache");
        if ( descriptor[i] ==  0x41)
            k_print_line("  L2 cache .......... 128 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x42)
            k_print_line("  L2 cache .......... 256 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x43)
            k_print_line("  L2 cache .......... 512 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x44)
            k_print_line("  L2 cache ..........   1 Mb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x45)
            k_print_line("  L2 cache ..........   2 Mb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x46)
            k_print_line("  L3 cache ..........   4 Mb, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x47)
            k_print_line("  L3 cache ..........   8 Mb, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x49)
            k_print_line("  L2 cache ..........   4 Mb, 16-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x50)
            k_print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 64 entries");
        if ( descriptor[i] ==  0x51)
            k_print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 128 entries");
        if ( descriptor[i] ==  0x52)
            k_print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 256 entries");
        if ( descriptor[i] ==  0x56)
            k_print_line("  Data TLB ..........   4 Mb pages, 4-way associative,  16 entries");
        if ( descriptor[i] ==  0x57)
            k_print_line("  Data TLB ..........   4 Kb pages, 4-way associative,  16 entries");
        if ( descriptor[i] ==  0x5B)
            k_print_line("  Data TLB ..........   4 kb and 4 Mb pages,  64 entries");
        if ( descriptor[i] ==  0x5C)
            k_print_line("  Data TLB ..........   4 kb and 4 Mb pages, 128 entries");
        if ( descriptor[i] ==  0x5D)
            k_print_line("  Data TLB ..........   4 kb and 4 Mb pages, 256 entries");
        if ( descriptor[i] ==  0x60)
            k_print_line("  L1 data cache .....  16 kb, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x66)
            k_print_line("  L1 data cache .....   8 kb, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x67)
            k_print_line("  L1 data cache .....  16 kb, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x68)
            k_print_line("  L1 data cache .....  32 kb, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x70)
            k_print_line("  Trace cache ......  12k uop, 8-way associative");
        if ( descriptor[i] ==  0x71)
            k_print_line("  Trace cache ......  16k uop, 8-way associative");
        if ( descriptor[i] ==  0x72)
            k_print_line("  Trace cache ......  32k uop, 8-way associative");
        if ( descriptor[i] ==  0x78)
            k_print_line("  L2 cache .......... 1 MB   , 8-way associative, 64byte line size");
        if ( descriptor[i] ==  0x79)
            k_print_line("  L2 cache .......... 128 kb, 8-way associative, sectored, 64 byte line size");
        if ( descriptor[i] ==  0x7A)
            k_print_line("  L2 cache .......... 256 kb, 8-way associative, sectored, 64 byte line size");
        if ( descriptor[i] ==  0x7B)
            k_print_line("  L2 cache .......... 512 kb, 8-way associative, sectored, 64 byte line size");
        if ( descriptor[i] ==  0x7C)
            k_print_line("  L2 cache .......... 1M Byte, 8-way associative, sectored, 64 byte line size");
        if ( descriptor[i] ==  0x7D)
            k_print_line("  L2 cache .......... 2M Byte, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x7F)
            k_print_line("  L2 cache .........512K Byte, 2-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x82)
            k_print_line("  L2 cache .......... 256 kb, 8-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x83)
            k_print_line("  L2 cache .......... 512K Byte, 8-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x84)
            k_print_line("  L2 cache ..........   1 Mb, 8-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x85)
            k_print_line("  L2 cache ..........   2 Mb, 8-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x86)
            k_print_line("  L2 cache ..........   512K Byte, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x87)
            k_print_line("  L2 cache ..........   1M Byte, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0xB0)
            k_print_line("  Instruction TLB       4K-Byte Pages, 4-way associative, 128 entries");
        if ( descriptor[i] ==  0xB3)
            k_print_line("  Data TLB               4K-Byte Pages, 4-way associative, 128 entries");
        if ( descriptor[i] ==  0xB4)
            k_print_line("  Data TLB               4K-Byte Pages, 4-way associative, 256 entries");
        if ( descriptor[i] ==  0xF0)
            k_print_line("  64-byte prefetching");
        if ( descriptor[i] ==  0xF1)
            k_print_line("  128-byte prefetching");
    }
}

void get_features(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    eax = 1;
    native_cpuid(&eax, &ebx, &ecx, &edx);

    k_print("Features:");

    test_feature(edx, FPU, "fpu");
    test_feature(edx, MMX, "mmx");
    test_feature(edx, SSE, "sse");
    test_feature(edx, SSE2, "sse2");
    test_feature(edx, HT, "ht");

    test_feature(ecx, SSE3, "sse3");
    test_feature(ecx, SSE41, "sse4_1");
    test_feature(ecx, SSE42, "sse4_2");
    test_feature(ecx, AES, "aes");
    test_feature(ecx, AVX, "avx");

    k_print_line();
}

void get_deterministic_cache_parameters(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    eax = 0;
    native_cpuid(&eax, &ebx, &ecx, &edx);

    if(eax < 4){
        //Not supported on this processor
        return;
    }

    size_t caches = 0;

    while(caches < 1000){
        eax = 4;
        native_cpuid(&eax, &ebx, &ecx, &edx);

        if ( (eax & 0x1F) == 0 ) {
            // No more caches
            break;
        }

        if ((eax & 0x1F) == 1){
            k_print("Data Cache:        ");
        }

        if ((eax & 0x1F) == 2){
            k_print("Instruction Cache: ");
        }

        if ((eax & 0x1F) == 3){
            k_print("Unified Cache:     ");
        }

        k_printf( "Level %d: ", (eax & 0xE0)/32 );
        k_printf( "Max Threads %d: ", ((eax & 0x03FFC000)/(8192))+1 );
        k_printf( "Max Procs. %d: " ,  ((eax & 0xFC000000)/(4*256*65536))+1 );
        k_printf( "Line Size = %d: ", (ebx & 0xFFF ) + 1 );
        k_printf( "Associativity = %d: ", ((ebx & 0xFFC00000)/4*16*65536) + 1 );
        k_printf( "Sets = %d:\n", ecx + 1 );

        ++caches;
    }
}

void sysinfo_command(const vector<string>&){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    eax = 1;
    native_cpuid(&eax, &ebx, &ecx, &edx);

    k_printf("Stepping: %d\n", eax & 0xF);
    k_printf("Model: %d\n", (eax >> 4) & 0xF);
    k_printf("Family: %d\n", (eax >> 8) & 0xF);
    k_printf("Processor Type: %d\n", (eax >> 12) & 0x3);
    k_printf("Extended Model: %d\n", (eax >> 16) & 0xF);
    k_printf("Extended Family: %d\n", (eax >> 20) & 0xFF);

    eax = 0;
    native_cpuid(&eax, &ebx, &ecx, &edx);

    char vendor_id[13];
    *(reinterpret_cast<uint32_t*>(vendor_id)+0) = ebx;
    *(reinterpret_cast<uint32_t*>(vendor_id)+1) = edx;
    *(reinterpret_cast<uint32_t*>(vendor_id)+2) = ecx;
    vendor_id[12] = '\0';

    k_printf("Vendor ID: %s\n", vendor_id);

    char brand_string[49];

    eax = 0x80000002;
    native_cpuid(&eax, &ebx, &ecx, &edx);
    *(reinterpret_cast<uint32_t*>(brand_string)+0) = eax;
    *(reinterpret_cast<uint32_t*>(brand_string)+1) = ebx;
    *(reinterpret_cast<uint32_t*>(brand_string)+2) = ecx;
    *(reinterpret_cast<uint32_t*>(brand_string)+3) = edx;

    eax = 0x80000003;
    native_cpuid(&eax, &ebx, &ecx, &edx);
    *(reinterpret_cast<uint32_t*>(brand_string)+4) = eax;
    *(reinterpret_cast<uint32_t*>(brand_string)+5) = ebx;
    *(reinterpret_cast<uint32_t*>(brand_string)+6) = ecx;
    *(reinterpret_cast<uint32_t*>(brand_string)+7) = edx;

    eax = 0x80000004;
    native_cpuid(&eax, &ebx, &ecx, &edx);
    *(reinterpret_cast<uint32_t*>(brand_string)+8) = eax;
    *(reinterpret_cast<uint32_t*>(brand_string)+9) = ebx;
    *(reinterpret_cast<uint32_t*>(brand_string)+10) = ecx;
    *(reinterpret_cast<uint32_t*>(brand_string)+11) = edx;

    brand_string[48] = '\0';

    k_printf("Brand String: %s\n", brand_string);

    get_features();
    get_cache_info();
    get_deterministic_cache_parameters();
}

} //end of anonymous namespace

void init_shell(){
    current_input_length = 0;
    history_index = 0;

    wipeout();

    k_print("thor> ");

    start_shell();
}
