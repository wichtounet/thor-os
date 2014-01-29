//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "scheduler.hpp"
#include "paging.hpp"
#include "assert.hpp"
#include "gdt.hpp"

#include "console.hpp"

#include "stl/array.hpp"

namespace {

struct process_control_t {
    scheduler::process_t process;
    scheduler::process_state state;
    size_t rounds;
};

std::array<process_control_t, scheduler::MAX_PROCESS> pcb;

bool started = false;

constexpr const size_t TURNOVER = 10;

size_t current_pid;
size_t next_pid = 0;

void idle_task(){
    while(true){
        k_print('a');
        asm volatile("hlt");
    }
}

char idle_stack[scheduler::user_stack_size];
char idle_kernel_stack[scheduler::kernel_stack_size];

void create_idle_task(){
    auto& idle_process = scheduler::new_process();

    idle_process.system = true;
    idle_process.physical_cr3 = paging::get_physical_pml4t();
    idle_process.paging_size = 0;

    idle_process.physical_user_stack = 0;
    idle_process.physical_kernel_stack = 0;

    idle_process.regs.rflags = 0x200;
    idle_process.regs.rip = reinterpret_cast<size_t>(&idle_task);
    idle_process.regs.rsp = reinterpret_cast<size_t>(&idle_stack[scheduler::user_stack_size - 1]);
    idle_process.kernel_rsp = reinterpret_cast<size_t>(&idle_kernel_stack[scheduler::kernel_stack_size - 1]);

    idle_process.regs.cs = gdt::LONG_SELECTOR;
    idle_process.regs.ds = gdt::DATA_SELECTOR;

    scheduler::queue_process(idle_process.pid);
}

void switch_to_process(const interrupt::syscall_regs& regs, size_t pid){
    current_pid = pid;

    k_printf("Switched to %u\n", current_pid);

    auto& process = pcb[current_pid];

    process.state = scheduler::process_state::RUNNING;

    gdt::tss.rsp0_low = process.process.kernel_rsp & 0xFFFFFFFF;
    gdt::tss.rsp0_high = process.process.kernel_rsp >> 32;

    auto stack_pointer = reinterpret_cast<uint64_t*>(regs.placeholder);

    *(stack_pointer + 4) = process.process.regs.ds;
    *(stack_pointer + 3) = process.process.regs.rsp;
    *(stack_pointer + 2) = process.process.regs.rflags;
    *(stack_pointer + 1) = process.process.regs.cs;
    *(stack_pointer + 0) = process.process.regs.rip;
    *(stack_pointer - 3) = process.process.regs.r12;
    *(stack_pointer - 4) = process.process.regs.r11;
    *(stack_pointer - 5) = process.process.regs.r10;
    *(stack_pointer - 6) = process.process.regs.r9;
    *(stack_pointer - 7) = process.process.regs.r8;
    *(stack_pointer - 8) = process.process.regs.rdi;
    *(stack_pointer - 9) = process.process.regs.rsi;
    *(stack_pointer - 10) = process.process.regs.rdx;
    *(stack_pointer - 11) = process.process.regs.rcx;
    *(stack_pointer - 12) = process.process.regs.rbx;
    *(stack_pointer - 13) = process.process.regs.rax;
    *(stack_pointer - 14) = process.process.regs.ds;

    asm volatile("mov cr3, %0" : : "r" (process.process.physical_cr3) : "memory");

    /*asm volatile("mov rax, %0; mov ds, ax; mov es, ax; mov fs, ax; mov gs, ax;"
        :  //No outputs
        : "r" (process.data_selector)
        : "rax");

    asm volatile("mov cr3, %0" : : "r" (process.physical_cr3) : "memory");

    asm volatile("push %0; push %1; pushfq; pop rax; or rax, 0x200; push rax; push %2; push %3; iretq"
        :  //No outputs
        : "r" (process.data_selector), "r" (process.user_rsp), "r" (process.code_selector), "r" (process.rip)
        : "rax", "memory");*/
}

size_t select_next_process(){
    auto next = (current_pid+ 1) % pcb.size();

    while(pcb[next].state != scheduler::process_state::READY){
        next = (next + 1) % pcb.size();
    }

    return next;
}

void save_context(const interrupt::syscall_regs& regs){
    auto& process = pcb[current_pid];

    process.process.regs = regs;
}

} //end of anonymous namespace

void scheduler::init(){
    //Create the idle task
    create_idle_task();
}

void scheduler::start(){
    started = true;

    current_pid = 0;
    pcb[current_pid].rounds = TURNOVER;
    pcb[current_pid].state = process_state::RUNNING;

    //Wait for the next interrupt
    while(true){
        asm volatile ("nop; nop; nop; nop");
    }
}

void scheduler::kill_current_process(const interrupt::syscall_regs& regs){
    k_printf("Kill %u\n", current_pid);

    //TODO At this point, memory should be released

    pcb[current_pid].state = scheduler::process_state::EMPTY;

    current_pid = (current_pid + 1) % scheduler::MAX_PROCESS;

    //Select the next process and switch to it
    auto index = select_next_process();
    switch_to_process(regs, index);
}

void scheduler::timer_reschedule(const interrupt::syscall_regs& regs){
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

        save_context(regs);

        switch_to_process(regs, pid);
    } else {
        ++process.rounds;
    }

    //At this point we just have to return to the current process
}

void scheduler::reschedule(const interrupt::syscall_regs& regs){
    thor_assert(started, "No interest in rescheduling before start");

    auto& process = pcb[current_pid];

    //The process just got blocked, choose another one
    if(process.state == process_state::BLOCKED){
        auto index = select_next_process();

        save_context(regs);

        switch_to_process(regs, index);
    }

    //At this point we just have to return to the current process
}

scheduler::process_t& scheduler::new_process(){
    auto pid = next_pid++;

    auto& process = pcb[pid];

    process.process.system = false;
    process.process.pid = pid;
    process.state = process_state::NEW;

    return process.process;
}

void scheduler::queue_process(scheduler::pid_t p){
    pcb[p].state = process_state::READY;
}

scheduler::pid_t scheduler::get_pid(){
    return current_pid;
}

void scheduler::block_process(pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pcb[pid].state == process_state::RUNNING, "Can only block RUNNING processes");

    pcb[pid].state = process_state::BLOCKED;
}

void scheduler::unblock_process(pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pcb[pid].state == process_state::BLOCKED, "Can only block BLOCKED processes");

    pcb[pid].state = process_state::READY;
}
