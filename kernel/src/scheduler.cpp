//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "stl/array.hpp"
#include "stl/vector.hpp"

#include "scheduler.hpp"
#include "paging.hpp"
#include "assert.hpp"
#include "gdt.hpp"
#include "terminal.hpp"

#include "console.hpp"

//Provided by task_switch.s
extern "C" {
extern void task_switch(size_t current, size_t next);
extern void init_task_switch(size_t current);
}

namespace {

struct process_control_t {
    scheduler::process_t process;
    scheduler::process_state state;
    size_t rounds;
};

//The Process Control Block
std::array<process_control_t, scheduler::MAX_PROCESS> pcb;

//Define one run queue for each priority level
std::array<std::vector<scheduler::pid_t>, scheduler::PRIORITY_LEVELS> run_queues;

bool started = false;

constexpr const size_t TURNOVER = 10;

size_t current_pid;
size_t next_pid = 0;

void idle_task(){
    while(true){
        asm volatile("hlt");
    }
}

char idle_stack[scheduler::user_stack_size];
char idle_kernel_stack[scheduler::kernel_stack_size];

void create_idle_task(){
    auto& idle_process = scheduler::new_process();

    idle_process.priority = scheduler::MIN_PRIORITY;

    idle_process.system = true;
    idle_process.physical_cr3 = paging::get_physical_pml4t();
    idle_process.paging_size = 0;

    idle_process.physical_user_stack = 0;
    idle_process.physical_kernel_stack = 0;

    auto rsp = &idle_stack[scheduler::user_stack_size - 1];
    rsp -= sizeof(interrupt::syscall_regs);

    idle_process.context = reinterpret_cast<interrupt::syscall_regs*>(rsp);

    idle_process.context->rflags = 0x200;
    idle_process.context->rip = reinterpret_cast<size_t>(&idle_task);
    idle_process.context->rsp = reinterpret_cast<size_t>(&idle_stack[scheduler::user_stack_size - 1] - sizeof(interrupt::syscall_regs) * 8);
    idle_process.context->cs = gdt::LONG_SELECTOR;
    idle_process.context->ds = gdt::DATA_SELECTOR;

    idle_process.kernel_rsp = reinterpret_cast<size_t>(&idle_kernel_stack[scheduler::kernel_stack_size - 1]);

    scheduler::queue_process(idle_process.pid);
}

void switch_to_process(size_t pid){
    auto old_pid = current_pid;
    current_pid = pid;

    k_printf("Switch to %u\n", current_pid);

    auto& process = pcb[current_pid];

    process.state = scheduler::process_state::RUNNING;

    gdt::tss.rsp0_low = process.process.kernel_rsp & 0xFFFFFFFF;
    gdt::tss.rsp0_high = process.process.kernel_rsp >> 32;

    task_switch(old_pid, current_pid);
}

size_t select_next_process(){
    auto current_priority = pcb[current_pid].process.priority;

    //1. Run a process of higher priority, if any
    for(size_t p = scheduler::MAX_PRIORITY; p > current_priority; --p){
        for(auto pid : run_queues[p]){
            if(pcb[pid].state == scheduler::process_state::READY){
                return pid;
            }
        }
    }

    //2. Run the next process of the same priority

    auto& current_run_queue = run_queues[current_priority];

    size_t next_index = 0;
    for(size_t i = 0; i < current_run_queue.size(); ++i){
        if(current_run_queue[i] == current_pid){
            next_index = (i + 1) % current_run_queue.size();
            break;
        }
    }

    for(size_t i = 0; i < current_run_queue.size(); ++i){
        auto index = (next_index + i) % current_run_queue.size();
        auto pid = current_run_queue[index];

        if(pcb[pid].state == scheduler::process_state::READY){
            return pid;
        }
    }

    thor_assert(current_priority > 0, "The idle task should always be ready");

    //3. Run a process of lower priority

    for(size_t p = current_priority - 1; p >= scheduler::MIN_PRIORITY; --p){
        for(auto pid : run_queues[p]){
            if(pcb[pid].state == scheduler::process_state::READY){
                return pid;
            }
        }
    }

    thor_unreachable("No process is READY");
}

} //end of anonymous namespace
void scheduler::init(){ //Create the idle task
    create_idle_task();
}

void scheduler::start(){
    started = true;

    current_pid = 0;
    pcb[current_pid].rounds = TURNOVER;
    pcb[current_pid].state = process_state::RUNNING;

    init_task_switch(current_pid);
}

void scheduler::kill_current_process(){
    k_printf("Kill %u\n", current_pid);

    //TODO At this point, memory should be released
    //TODO The process should also be removed from the run queue

    pcb[current_pid].state = scheduler::process_state::EMPTY;

    current_pid = (current_pid + 1) % scheduler::MAX_PROCESS;

    //Select the next process and switch to it
    auto index = select_next_process();
    switch_to_process(index);
}

void scheduler::timer_reschedule(){
    if(!started){
        return;
    }

    auto& process = pcb[current_pid];

    if(process.rounds  == TURNOVER){
        process.rounds = 0;

        process.state = process_state::READY;

        auto pid = select_next_process();

        //If it is the same, no need to go to the switching process
        if(pid == current_pid){
            return;
        }

        switch_to_process(pid);
    } else {
        ++process.rounds;
    }

    //At this point we just have to return to the current process
}

void scheduler::reschedule(){
    thor_assert(started, "No interest in rescheduling before start");

    auto& process = pcb[current_pid];

    //The process just got blocked, choose another one
    if(process.state == process_state::BLOCKED){
        auto index = select_next_process();

        switch_to_process(index);
    }

    //At this point we just have to return to the current process
}

scheduler::process_t& scheduler::new_process(){
    //TODO use get_free_pid() that searchs through the PCB
    auto pid = next_pid++;

    auto& process = pcb[pid];

    process.process.system = false;
    process.process.pid = pid;
    process.process.priority = scheduler::DEFAULT_PRIORITY;
    process.state = process_state::NEW;
    process.process.tty = stdio::get_active_terminal().id;

    return process.process;
}

void scheduler::queue_process(scheduler::pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");

    auto& process = pcb[pid];

    thor_assert(process.process.priority <= scheduler::MAX_PRIORITY, "Invalid priority");
    thor_assert(process.process.priority >= scheduler::MIN_PRIORITY, "Invalid priority");

    process.state = process_state::READY;

    run_queues[process.process.priority].push_back(pid);
}

scheduler::pid_t scheduler::get_pid(){
    return current_pid;
}

scheduler::process_t& scheduler::get_process(pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");

    return pcb[pid].process;
}

void scheduler::block_process(pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pcb[pid].state == process_state::RUNNING, "Can only block RUNNING processes");

    k_printf("Block process %u\n", pid);

    pcb[pid].state = process_state::BLOCKED;

    reschedule();
}

void scheduler::unblock_process(pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pcb[pid].state == process_state::BLOCKED, "Can only block BLOCKED processes");

    k_printf("Unblock process %u\n", pid);

    pcb[pid].state = process_state::READY;
}

//Provided for task_switch.s

extern "C" {

uint64_t get_context_address(size_t pid){
    return reinterpret_cast<uint64_t>(&pcb[pid].process.context);
}

uint64_t get_process_cr3(size_t pid){
    return reinterpret_cast<uint64_t>(pcb[pid].process.physical_cr3);
}

} //end of extern "C"
