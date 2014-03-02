//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <types.hpp>
#include <algorithms.hpp>
#include <vector.hpp>
#include <string.hpp>
#include <optional.hpp>

#include "shell.hpp"
#include "keyboard.hpp"
#include "kernel_utils.hpp"
#include "console.hpp"
#include "timer.hpp"
#include "disks.hpp"
#include "ata.hpp"
#include "acpi.hpp"
#include "rtc.hpp"
#include "elf.hpp"
#include "paging.hpp"
#include "gdt.hpp"
#include "process.hpp"
#include "scheduler.hpp"

#include "physical_allocator.hpp"
#include "virtual_allocator.hpp"
#include "kalloc.hpp"
#include "e820.hpp"

namespace {

#ifdef CONFIG_HISTORY
static constexpr const bool History = true;
#else
static constexpr const bool History = false;
#endif

std::vector<std::string> history;
uint64_t history_index = 0;

bool shift = false;

//Declarations of the different functions

void help_command(const std::vector<std::string>& params);
void clear_command(const std::vector<std::string>& params);
void memory_command(const std::vector<std::string>& params);
void disks_command(const std::vector<std::string>& params);
void partitions_command(const std::vector<std::string>& params);
void exec_command(const std::vector<std::string>& params);
void paginginfo_command(const std::vector<std::string>& params);

struct command_definition {
    const char* name;
    void (*function)(const std::vector<std::string>&);
};

command_definition commands[7] = {
    {"help", help_command},
    {"clear", clear_command},
    {"memory", memory_command},
    {"disks", disks_command},
    {"partitions", partitions_command},
    {"exec", exec_command},
    {"paginginfo", paginginfo_command},
};

std::string current_input(16);

void exec_shell_command();

template<bool Enable = History>
void history_key(char key){
    if(history.size() > 0){
        if(key == keyboard::KEY_UP){
            if(history_index == 0){
                return;
            }

            --history_index;
        } else { //KEY_DOWN
            if(history_index == history.size()){
                return;
            }

            ++history_index;
        }

        set_column(6);

        for(uint64_t i = 0; i < current_input.size(); ++i){
            k_print(' ');
        }

        set_column(6);

        if(history_index < history.size()){
            current_input = history[history_index];
        }

        k_print(current_input);
    }
}

template<> void history_key<false>(char){}

template<bool Enable = History>
void history_save(){
    history.push_back(current_input);
    history_index = history.size();
}

template<> void history_save<false>(){}

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
                if(current_input.size() > 0){
                    exec_shell_command();

                    if(get_column() != 0){
                        k_print_line();
                    }

                    current_input.clear();
                }

                k_print("thor> ");
            } else if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                shift = true;
            } else if(key == keyboard::KEY_UP || key == keyboard::KEY_DOWN){
                history_key(key);
            } else if(key == keyboard::KEY_BACKSPACE){
                if(current_input.size() > 0){
                    current_input.pop_back();
                }
            } else {
                auto qwertz_key =
                    shift
                    ? keyboard::shift_key_to_ascii(key)
                    : keyboard::key_to_ascii(key);

                if(qwertz_key){
                    current_input += qwertz_key;
                }
            }
        }
    }
}

void exec_shell_command(){
    history_save();

    auto params = std::split(current_input);;

    for(auto& command : commands){
        if(params[0] == command.name){
            command.function(params);

            return;
        }
    }

    k_printf("The command \"%s\" does not exist\n", current_input.c_str());
}

void clear_command(const std::vector<std::string>&){
    wipeout();
}

void help_command(const std::vector<std::string>&){
    k_print("Available commands:\n");

    for(auto& command : commands){
        k_print('\t');
        k_print_line(command.name);
    }
}

void memory_command(const std::vector<std::string>&){
    k_print_line("Physical:");
    k_printf("\tAvailable: %m (%h)\n", physical_allocator::available(), physical_allocator::available());
    k_printf("\tAllocated: %m (%h)\n", physical_allocator::allocated(), physical_allocator::allocated());
    k_printf("\tFree: %m (%h)\n", physical_allocator::free(), physical_allocator::free());

    k_print_line("Virtual:");
    k_printf("\tAvailable: %m (%h)\n", virtual_allocator::available(), virtual_allocator::available());
    k_printf("\tAllocated: %m (%h)\n", virtual_allocator::allocated(), virtual_allocator::allocated());
    k_printf("\tFree: %m (%h)\n", virtual_allocator::free(), virtual_allocator::free());

    k_print_line("Dynamic:");
    k_printf("\tAllocated: %m (%h)\n", kalloc::allocated_memory(), kalloc::allocated_memory());
    k_printf("\tUsed: %m (%h)\n", kalloc::used_memory(), kalloc::used_memory());
    k_printf("\tFree: %m (%h)\n", kalloc::free_memory(), kalloc::free_memory());
}

void disks_command(const std::vector<std::string>& params){
    bool verbose = false;

    //Read options if any
    if(params.size() > 1){
        for(size_t i = 1; i < params.size(); ++i){
            if(params[i] == "-v"){
                verbose = true;
            }
        }
    }

    if(verbose){
        k_print_line("UUID       Type  Model                Serial          Firmware");
    } else {
        k_print_line("UUID       Type");
    }

    for(uint64_t i = 0; i < disks::detected_disks(); ++i){
        auto& descriptor = disks::disk_by_index(i);

        if(verbose){
            if(descriptor.type == disks::disk_type::ATA || descriptor.type == disks::disk_type::ATAPI){
                auto sub = static_cast<ata::drive_descriptor*>(descriptor.descriptor);

                k_printf("%10d %5s %20s %15s %s\n", descriptor.uuid, disks::disk_type_to_string(descriptor.type),
                    sub->model.c_str(), sub->serial.c_str(), sub->firmware.c_str());
            } else {
                k_printf("%10d %s\n", descriptor.uuid, disks::disk_type_to_string(descriptor.type));
            }
        } else {
            k_printf("%10d %s\n", descriptor.uuid, disks::disk_type_to_string(descriptor.type));
        }
    }
}

void partitions_command(const std::vector<std::string>& params){
    auto uuid = parse(params[1]);

    if(disks::disk_exists(uuid)){
        auto& disk = disks::disk_by_uuid(uuid);

        if(disk.type != disks::disk_type::ATA){
            k_print_line("Only ATA disks are supported");
        } else {
            auto partitions = disks::partitions(disk);

            if(partitions.size() > 0){
                k_print_line("UUID       Type         Start      Sectors");

                for(auto& partition : partitions){
                    k_printf("%10d %12s %10d %u\n", partition.uuid,
                        disks::partition_type_to_string(partition.type),
                        partition.start, partition.sectors);
                }
            }
        }
    } else {
        k_printf("Disks %u does not exist\n", uuid);
    }
}

void exec_command(const std::vector<std::string>&){
    //Fake exec just to start() the scheduler
    std::vector<std::string> params;
    scheduler::exec("", params);
}

void paginginfo_command(const std::vector<std::string>&){
    k_printf("Page Size: %u\n", paging::PAGE_SIZE);
    k_printf("Number of PDPT: %u\n", paging::pml4_entries);
    k_printf("Number of PD: %u\n", paging::pdpt_entries);
    k_printf("Number of PT: %u\n", paging::pd_entries);
    k_printf("Virtual Start: %h\n", paging::virtual_paging_start);
    k_printf("Physical Size: %h\n", paging::physical_memory_pages * paging::PAGE_SIZE);
}

} //end of anonymous namespace

void init_shell(){
    wipeout();

    k_print("thor> ");

    start_shell();
}
