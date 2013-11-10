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

namespace {

//Declarations of the different functions

void reboot_command(const char* params);
void help_command(const char* params);
void uptime_command(const char* params);
void clear_command(const char* params);
void date_command(const char* params);
void sleep_command(const char* params);
void echo_command(const char* params);
void mmap_command(const char* params);
void memory_command(const char* params);
void disks_command(const char* params);
void partitions_command(const char* params);
void mount_command(const char* params);
void ls_command(const char* params);
void free_command(const char* params);

struct command_definition {
    const char* name;
    void (*function)(const char*);
};

command_definition commands[14] = {
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
    {"ls", ls_command},
    {"free", free_command},
};

uint64_t current_input_length = 0;
char current_input[50];

void exec_command();

void start_shell(){
    while(true){
        auto key = keyboard::get_char();

        if(key & 0x80){
            //TODO Handle shift
        } else {
            if(key == keyboard::KEY_ENTER){
                current_input[current_input_length] = '\0';

                k_print_line();

                exec_command();

                if(get_column() != 0){
                    set_column(0);
                    set_line(get_line() + 1);
                }

                current_input_length = 0;

                k_print("thor> ");
            } else if(key == keyboard::KEY_BACKSPACE){
                if(current_input_length > 0){
                    set_column(get_column() - 1);
                    k_print(' ');
                    set_column(get_column() - 1);

                    --current_input_length;
                }
            } else {
                auto qwertz_key = keyboard::key_to_ascii(key);

                if(qwertz_key > 0){
                    current_input[current_input_length++] = qwertz_key;
                    k_print(qwertz_key);
                }
            }
        }
    }
}

void exec_command(){
    char buffer[50];

    for(auto& command : commands){
        const char* input_command = current_input;
        if(str_contains(current_input, ' ')){
            str_copy(current_input, buffer);
            input_command = str_until(buffer, ' ');
        }

        if(str_equals(input_command, command.name)){
            command.function(current_input);

            return;
        }
    }

    k_printf("The command \"%s\" does not exist\n", current_input);
}

void clear_command(const char*){
    wipeout();
}

void reboot_command(const char*){
    interrupt<60>();
}

void help_command(const char*){
    k_print("Available commands:\n");

    for(auto& command : commands){
        k_print('\t');
        k_print_line(command.name);
    }
}

void uptime_command(const char*){
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

void date_command(const char*){
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

void sleep_command(const char* params){
    const char* delay_str = params + 6;

    sleep_ms(parse(delay_str) * 1000);
}

void echo_command(const char* params){
    k_print_line(params + 5);
}

void mmap_command(const char*){
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

void memory_command(const char*){
    if(mmap_failed()){
        k_print_line("The mmap was not correctly loaded from e820");
    } else {
        k_printf("Total available memory: %m\n", available_memory());
        k_printf("Total used memory: %m\n", used_memory());
        k_printf("Total free memory: %m\n", free_memory());
        k_printf("Total allocated memory: %m\n", allocated_memory());
    }
}

void disks_command(const char*){
    k_print_line("UUID       Type");

    for(uint64_t i = 0; i < disks::detected_disks(); ++i){
        auto& descriptor = disks::disk_by_index(i);

        k_printf("%10d %s\n", descriptor.uuid, disks::disk_type_to_string(descriptor.type));
    }
}

void partitions_command(const char* params){
    const char* delay_str = params + 11;

    auto uuid = parse(delay_str);

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

void mount_command(const char* params){
    if(!*(params+5)){
        auto md = disks::mounted_disk();
        auto mp = disks::mounted_partition();

        if(md && mp){
            k_printf("%d:%d is mounted\n", md->uuid, mp->uuid);
        } else {
            k_print_line("Nothing is mounted");
        }
    } else {
        const char* it = params + 6;
        const char* it_end = it;

        while(*it_end != ' '){
            ++it_end;
        }

        auto disk_uuid = parse(it, it_end);
        auto partition_uuid = parse(it_end + 1);

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

void ls_command(const char*){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        k_print_line("Nothing is mounted");

        return;
    }

    auto files = disks::ls();

    for(auto& file : files){
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

void free_command(const char*){
    if(!disks::mounted_partition() || !disks::mounted_disk()){
        k_print_line("Nothing is mounted");

        return;
    }

    k_printf("Free size: %m\n", disks::free_size());
}

} //end of anonymous namespace

void init_shell(){
    current_input_length = 0;

    clear_command(0);

    k_print("thor> ");

    start_shell();
}
