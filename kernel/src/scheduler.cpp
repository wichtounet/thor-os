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

namespace {

bool started = false;

std::vector<scheduler::process_t> processes;
std::vector<size_t> rounds;

constexpr const size_t TURNOVER = 10;

size_t current_index;

size_t next_pid = 1;

void idle_task(){
    while(true){
        k_print('a');
        asm volatile("hlt");
    }
}

size_t idle_stack[64];
size_t idle_kernel_stack[4096]; //TODO Perhaps not good

void create_idle_task(){
    auto idle_process = scheduler::new_process();
    idle_process.system = true;
    idle_process.physical_cr3 = paging::get_physical_pml4t();
    idle_process.paging_size = 0;

    idle_process.physical_user_stack = 0;
    idle_process.physical_kernel_stack = 0;

    idle_process.regs.rflags = 0x200;
    idle_process.regs.rip = reinterpret_cast<size_t>(&idle_task);
    idle_process.regs.rsp = reinterpret_cast<size_t>(&idle_stack[63]);
    idle_process.kernel_rsp = reinterpret_cast<size_t>(&idle_kernel_stack[4095]);

    idle_process.regs.cs = gdt::LONG_SELECTOR;
    idle_process.regs.ds = gdt::DATA_SELECTOR;

    queue_process(std::move(idle_process));
}

void switch_to_process(const interrupt::syscall_regs& regs, size_t index){
    current_index = index;

    k_printf("Switched to %u\n", index);

    auto& process = processes[current_index];

    process.state = scheduler::process_state::RUNNING;

    gdt::tss.rsp0_low = process.kernel_rsp & 0xFFFFFFFF;
    gdt::tss.rsp0_high = process.kernel_rsp >> 32;

    auto stack_pointer = reinterpret_cast<uint64_t*>(regs.placeholder);

    *(stack_pointer + 4) = process.regs.ds;
    *(stack_pointer + 3) = process.regs.rsp;
    *(stack_pointer + 2) = process.regs.rflags;
    *(stack_pointer + 1) = process.regs.cs;
    *(stack_pointer + 0) = process.regs.rip;
    *(stack_pointer - 3) = process.regs.r12;
    *(stack_pointer - 4) = process.regs.r11;
    *(stack_pointer - 5) = process.regs.r10;
    *(stack_pointer - 6) = process.regs.r9;
    *(stack_pointer - 7) = process.regs.r8;
    *(stack_pointer - 8) = process.regs.rdi;
    *(stack_pointer - 9) = process.regs.rsi;
    *(stack_pointer - 10) = process.regs.rdx;
    *(stack_pointer - 11) = process.regs.rcx;
    *(stack_pointer - 12) = process.regs.rbx;
    *(stack_pointer - 13) = process.regs.rax;
    *(stack_pointer - 14) = process.regs.ds;

    asm volatile("mov cr3, %0" : : "r" (process.physical_cr3) : "memory");

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
    auto next = (current_index + 1) % processes.size();

    while(processes[next].state != scheduler::process_state::READY){
        next = (next + 1) % processes.size();
    }

    return next;
}

void save_context(const interrupt::syscall_regs& regs){
    auto& process = processes[current_index];

    process.regs = regs;
}

} //end of anonymous namespace

void scheduler::init(){
    //Create the idle task
    create_idle_task();
}

void scheduler::start(){
    thor_assert(!processes.empty(), "There should at least be the idle task");

    started = true;

    current_index = 0;
    rounds[current_index] = TURNOVER;
    processes[current_index].state = process_state::RUNNING;

    //Wait for the next interrupt
    while(true){
        asm volatile ("nop; nop; nop; nop");
    }
}

void scheduler::kill_current_process(const interrupt::syscall_regs& regs){
    k_printf("Kill %u\n", current_index);

    processes.erase(current_index);
    rounds.erase(current_index);

    //TODO At this point, memory should be released

    //Start from the first again
    current_index = 0;

    //Select the next process and switch to it
    auto index = select_next_process();
    switch_to_process(regs, index);
}

void scheduler::reschedule(const interrupt::syscall_regs& regs){
    if(!started){
        return;
    }

    if(rounds[current_index] == TURNOVER){
        rounds[current_index] = 0;

        processes[current_index].state = process_state::READY;

        auto index = select_next_process();

        //If it is the same, no need to go to the switching process
        if(index == current_index){
            return;
        }

        save_context(regs);

        switch_to_process(regs, index);
    } else {
        ++rounds[current_index];
    }

    //At this point we just have to return to the current process
}

scheduler::process_t scheduler::new_process(){
    process_t p;

    p.system = false;
    p.pid = next_pid++;
    p.state = process_state::NEW;

    return std::move(p);
}

void scheduler::queue_process(process_t&& p){
    processes.push_back(std::forward<scheduler::process_t>(p));
    rounds.push_back(0);

    processes.back().state = process_state::READY;
}
