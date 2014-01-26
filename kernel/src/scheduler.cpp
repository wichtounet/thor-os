//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "scheduler.hpp"
#include "process.hpp"
#include "paging.hpp"
#include "assert.hpp"

#include "console.hpp"

namespace {

std::vector<scheduler::process_t> processes;
std::vector<size_t> rounds;

constexpr const size_t TURNOVER = 10;

size_t current_index;

size_t next_pid = 1;

void idle_task(){
    while(true){
        asm volatile("hlt");
    }
}

size_t idle_stack[64];
size_t idle_kernel_stack[4096]; //TODO Perhaps not good

void create_idle_task(){
    scheduler::process_t idle_process;
    idle_process.pid = next_pid++;
    idle_process.system = true;
    idle_process.physical_cr3 = paging::get_physical_pml4t();
    idle_process.paging_size = 0;

    idle_process.physical_user_stack = 0;
    idle_process.physical_kernel_stack = 0;

    idle_process.rip = reinterpret_cast<size_t>(&idle_task);
    idle_process.user_rsp = reinterpret_cast<size_t>(&idle_stack[63]);
    idle_process.kernel_rsp = reinterpret_cast<size_t>(&idle_kernel_stack[4095]);

    processes.push_back(std::move(idle_process));
    rounds.push_back(0);
}

void switch_to_process(size_t index){
    current_index = index;

    k_printf("Switched to %u\n", index);

    //TODO
}

size_t select_next_process(){
    return (current_index + 1) % processes.size();
}

} //end of anonymous namespace

void scheduler::init(){
    //Create the idle task
    create_idle_task();
}

void scheduler::start(){
    thor_assert(!processes.empty(), "There should at least be the idle task");

    switch_to_process(processes.size() - 1);
}

void scheduler::kill_current_process(){
    k_printf("Kill %u\n", current_index);

    processes.erase(current_index);
    rounds.erase(current_index);

    current_index = processes.size();

    //TODO At this point, memory should be released

    reschedule();
}

void scheduler::reschedule(){
    //Test if the current process just got killed
    if(current_index == processes.size()){
        current_index = 0;

        auto index = select_next_process();
        switch_to_process(index);
    }

    if(rounds[current_index] == TURNOVER){
        rounds[current_index] = 0;

        auto index = select_next_process();
        switch_to_process(index);
    } else {
        ++rounds[current_index];
    }

    //At this point we just have to return to the current process
}
