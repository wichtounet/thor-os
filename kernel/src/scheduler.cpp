//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <array.hpp>
#include <vector.hpp>
#include <optional.hpp>
#include <string.hpp>
#include <lock_guard.hpp>
#include <errors.hpp>

#include "scheduler.hpp"
#include "paging.hpp"
#include "assert.hpp"
#include "gdt.hpp"
#include "terminal.hpp"
#include "disks.hpp"
#include "elf.hpp"
#include "console.hpp"
#include "physical_allocator.hpp"
#include "virtual_allocator.hpp"
#include "physical_pointer.hpp"
#include "mutex.hpp"

constexpr const bool DEBUG_SCHEDULER = false;

//Provided by task_switch.s
extern "C" {
extern void task_switch(size_t current, size_t next);
extern void init_task_switch(size_t current) __attribute__((noreturn));
}

namespace {

struct process_control_t {
    scheduler::process_t process;
    scheduler::process_state state;
    size_t rounds;
    size_t sleep_timeout;
    std::vector<std::string> handles;
};

//The Process Control Block
std::array<process_control_t, scheduler::MAX_PROCESS> pcb;

//Define one run queue for each priority level
std::array<std::vector<scheduler::pid_t>, scheduler::PRIORITY_LEVELS> run_queues;
std::array<mutex, scheduler::PRIORITY_LEVELS> run_queue_locks;

std::vector<scheduler::pid_t>& run_queue(size_t priority){
    return run_queues[priority - scheduler::MIN_PRIORITY];
}

mutex& run_queue_lock(size_t priority){
    return run_queue_locks[priority - scheduler::MIN_PRIORITY];
}

bool started = false;

constexpr const size_t TURNOVER = 10;

constexpr const size_t QUANTUM_SIZE = 1000;
size_t current_ticks = 0;

size_t current_pid;
size_t next_pid = 0;

size_t gc_pid = 0;

void idle_task(){
    while(true){
        asm volatile("hlt");
    }
}

void gc_task(){
    while(true){
        //Wait until there is something to do
        scheduler::block_process(scheduler::get_pid());

        //2. Clean up each killed process

        for(auto& process : pcb){
            if(process.state == scheduler::process_state::KILLED){
                auto& desc = process.process;

                if(DEBUG_SCHEDULER){
                    k_printf("Clean process %u\n", desc.pid);
                }

                //1. Release physical memory of PML4T
                physical_allocator::free(desc.physical_cr3, 1);

                //3. Release physical stacks
                physical_allocator::free(desc.physical_kernel_stack, scheduler::kernel_stack_size / paging::PAGE_SIZE);
                physical_allocator::free(desc.physical_user_stack, scheduler::user_stack_size / paging::PAGE_SIZE);

                //4. Release segment's physical memory
                for(auto& segment : desc.segments){
                    physical_allocator::free(segment.physical, segment.size / paging::PAGE_SIZE);
                }
                desc.segments.clear();

                //5. Release virtual kernel stack
                virtual_allocator::free(desc.virtual_kernel_stack, scheduler::kernel_stack_size / paging::PAGE_SIZE);
                paging::unmap_pages(desc.virtual_kernel_stack, scheduler::kernel_stack_size / paging::PAGE_SIZE);

                //6. Remove process from run queue
                size_t index = 0;
                for(; index < run_queue(desc.priority).size(); ++index){
                    std::lock_guard<mutex> l(run_queue_lock(desc.priority));

                    if(run_queue(desc.priority)[index] == desc.pid){
                        run_queue(desc.priority).erase(index);
                        break;
                    }
                }

                //7. Clean process
                desc.pid = 0;
                desc.ppid = 0;
                desc.system = false;
                desc.physical_cr3 = 0;
                desc.physical_user_stack = 0;
                desc.physical_kernel_stack = 0;
                desc.paging_size = 0;
                desc.context = nullptr;
                desc.brk_start = desc.brk_end = 0;

                //8 Clean file handles
                //TODO If not empty, probably something should be done
                process.handles.clear();

                //9. Release the PCB slot
                process.state = scheduler::process_state::EMPTY;
            }
        }
    }
}

//TODO tsh should be configured somewhere
void init_task(){
    std::vector<std::string> params;

    while(true){
        auto pid = scheduler::exec("tsh", params);
        scheduler::await_termination(pid);

        if(DEBUG_SCHEDULER){
            k_print_line("shell exited, run new one");
        }
    }
}

char idle_stack[scheduler::user_stack_size];
char idle_kernel_stack[scheduler::kernel_stack_size];

char init_stack[scheduler::user_stack_size];
char init_kernel_stack[scheduler::kernel_stack_size];

char gc_stack[scheduler::user_stack_size];
char gc_kernel_stack[scheduler::kernel_stack_size];

scheduler::process_t& new_process(){
    //TODO use get_free_pid() that searchs through the PCB
    auto pid = next_pid++;

    auto& process = pcb[pid];

    process.process.system = false;
    process.process.pid = pid;
    process.process.ppid = current_pid;
    process.process.priority = scheduler::DEFAULT_PRIORITY;
    process.state = scheduler::process_state::NEW;
    process.process.tty = stdio::get_active_terminal().id;

    process.process.brk_start = 0;
    process.process.brk_end = 0;

    return process.process;
}

void queue_process(scheduler::pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");

    auto& process = pcb[pid];

    thor_assert(process.process.priority <= scheduler::MAX_PRIORITY, "Invalid priority");
    thor_assert(process.process.priority >= scheduler::MIN_PRIORITY, "Invalid priority");

    process.state = scheduler::process_state::READY;

    std::lock_guard<mutex> l(run_queue_lock(process.process.priority));
    run_queue(process.process.priority).push_back(pid);
}

scheduler::process_t& create_kernel_task(char* user_stack, char* kernel_stack, void (*fun)()){
    auto& process = new_process();

    process.system = true;
    process.physical_cr3 = paging::get_physical_pml4t();
    process.paging_size = 0;

    process.physical_user_stack = 0;
    process.physical_kernel_stack = 0;

    auto rsp = &user_stack[scheduler::user_stack_size - 1];
    rsp -= sizeof(interrupt::syscall_regs);

    process.context = reinterpret_cast<interrupt::syscall_regs*>(rsp);

    process.context->rflags = 0x200;
    process.context->rip = reinterpret_cast<size_t>(fun);
    process.context->rsp = reinterpret_cast<size_t>(&user_stack[scheduler::user_stack_size - 1] - sizeof(interrupt::syscall_regs) * 8);
    process.context->cs = gdt::LONG_SELECTOR;
    process.context->ds = gdt::DATA_SELECTOR;

    process.kernel_rsp = reinterpret_cast<size_t>(&kernel_stack[scheduler::kernel_stack_size - 1]);

    return process;
}

void create_idle_task(){
    auto& idle_process = create_kernel_task(idle_stack, idle_kernel_stack, &idle_task);

    idle_process.ppid = 0;
    idle_process.priority = scheduler::MIN_PRIORITY;

    queue_process(idle_process.pid);
}

void create_init_task(){
    auto& init_process = create_kernel_task(init_stack, init_kernel_stack, &init_task);

    init_process.ppid = 0;
    init_process.priority = scheduler::MIN_PRIORITY + 1;

    queue_process(init_process.pid);
}

void create_gc_task(){
    auto& gc_process = create_kernel_task(gc_stack, gc_kernel_stack, &gc_task);

    gc_process.ppid = 1;
    gc_process.priority = scheduler::MIN_PRIORITY + 1;

    queue_process(gc_process.pid);

    gc_pid = gc_process.pid;
}

void switch_to_process(size_t pid){
    auto old_pid = current_pid;
    current_pid = pid;

    if(DEBUG_SCHEDULER){
        k_printf("Switch to %u\n", current_pid);
    }

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
        std::lock_guard<mutex> l(run_queue_lock(p));

        for(auto pid : run_queue(p)){
            if(pcb[pid].state == scheduler::process_state::READY){
                return pid;
            }
        }
    }

    //2. Run the next process of the same priority

    {
        std::lock_guard<mutex> l(run_queue_lock(current_priority));

        auto& current_run_queue = run_queue(current_priority);

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
    }

    thor_assert(current_priority > 0, "The idle task should always be ready");

    //3. Run a process of lower priority

    for(size_t p = current_priority - 1; p >= scheduler::MIN_PRIORITY; --p){
        std::lock_guard<mutex> l(run_queue_lock(p));

        for(auto pid : run_queue(p)){
            if(pcb[pid].state == scheduler::process_state::READY){
                return pid;
            }
        }
    }

    thor_unreachable("No process is READY");
}

bool allocate_user_memory(scheduler::process_t& process, size_t address, size_t size, size_t& ref){
    //1. Calculate some stuff
    auto first_page = paging::page_align(address);
    auto left_padding = address - first_page;

    auto bytes = left_padding + size;

    //Make sure only complete pages are allocated
    if(bytes % paging::PAGE_SIZE != 0){
        bytes += paging::PAGE_SIZE - (bytes % paging::PAGE_SIZE);
    }

    auto pages = bytes / paging::PAGE_SIZE;

    //2. Get enough physical memory
    auto physical_memory = physical_allocator::allocate(pages);

    if(!physical_memory){
        k_print_line("Cannot allocate physical memory, probably out of memory");
        return false;
    }

    //3. Find a start of a page inside the physical memory

    auto aligned_physical_memory = paging::page_aligned(physical_memory) ? physical_memory :
            (physical_memory / paging::PAGE_SIZE + 1) * paging::PAGE_SIZE;

    if(DEBUG_SCHEDULER){
        k_printf("Map(p%u) virtual:%h into phys: %h\n", process.pid, first_page, aligned_physical_memory);
    }


    //4. Map physical allocated memory to the necessary virtual memory
    if(!paging::user_map_pages(process, first_page, aligned_physical_memory, pages)){
        if(DEBUG_SCHEDULER){
            k_print_line("Impossible to map in user space");
        }

        return false;
    }

    ref = physical_memory;

    return true;
}

void clear_physical_memory(size_t memory, size_t pages){
    physical_pointer phys_ptr(memory, pages);

    auto it = phys_ptr.as_ptr<uint64_t>();
    std::fill_n(it, (pages * paging::PAGE_SIZE) / sizeof(uint64_t), 0);
}

bool create_paging(char* buffer, scheduler::process_t& process){
    //1. Prepare PML4T

    //Get memory for cr3
    process.physical_cr3 = physical_allocator::allocate(1);
    process.paging_size = paging::PAGE_SIZE;

    if(DEBUG_SCHEDULER){
        k_printf("Process %u cr3:%h\n", process.pid, process.physical_cr3);
    }

    clear_physical_memory(process.physical_cr3, 1);

    //Map the kernel pages inside the user memory space
    paging::map_kernel_inside_user(process);

    //2. Create all the other necessary structures

    //2.1 Allocate user stack
    allocate_user_memory(process, scheduler::user_stack_start, scheduler::user_stack_size, process.physical_user_stack);

    //2.2 Allocate all user segments

    auto header = reinterpret_cast<elf::elf_header*>(buffer);
    auto program_header_table = reinterpret_cast<elf::program_header*>(buffer + header->e_phoff);

    for(size_t p = 0; p < header->e_phnum; ++p){
        auto& p_header = program_header_table[p];

        if(p_header.p_type == 1){
            auto first_page = p_header.p_vaddr;
            auto left_padding = p_header.p_vaddr - first_page;

            auto bytes = left_padding + p_header.p_memsz;

            //Make sure only complete pages are allocated
            if(bytes % paging::PAGE_SIZE != 0){
                bytes += paging::PAGE_SIZE - (bytes % paging::PAGE_SIZE);
            }

            auto pages = bytes / paging::PAGE_SIZE;

            scheduler::segment_t segment;
            segment.size = bytes;

            allocate_user_memory(process, first_page, bytes, segment.physical);

            process.segments.push_back(segment);

            //Copy the code into memory

            physical_pointer phys_ptr(segment.physical, pages);

            if(DEBUG_SCHEDULER){
                k_printf("Copy to physical:%h\n", segment.physical);
            }

            auto memory_start = phys_ptr.get() + left_padding;

            //In the case of the BSS segment, the segment must be
            //filled with zero
            if(p_header.p_filesize != p_header.p_memsz){
                std::copy_n(reinterpret_cast<char*>(memory_start), buffer + p_header.p_offset, p_header.p_filesize);
                std::fill_n(reinterpret_cast<char*>(memory_start + p_header.p_filesize), p_header.p_memsz - p_header.p_filesize, 0);
            } else {
                std::copy_n(reinterpret_cast<char*>(memory_start), buffer + p_header.p_offset, p_header.p_memsz);
            }
        }
    }

    //2.3 Allocate kernel stack
    auto virtual_kernel_stack = virtual_allocator::allocate(scheduler::kernel_stack_size / paging::PAGE_SIZE);
    auto physical_kernel_stack = physical_allocator::allocate(scheduler::kernel_stack_size / paging::PAGE_SIZE);

    if(!paging::map_pages(virtual_kernel_stack, physical_kernel_stack, scheduler::kernel_stack_size / paging::PAGE_SIZE)){
        return false;
    }

    process.physical_kernel_stack = physical_kernel_stack;
    process.virtual_kernel_stack = virtual_kernel_stack;
    process.kernel_rsp = virtual_kernel_stack + (scheduler::user_stack_size - 8);

    //3. Clear stacks
    clear_physical_memory(process.physical_user_stack, scheduler::user_stack_size / paging::PAGE_SIZE);
    clear_physical_memory(process.physical_kernel_stack, scheduler::kernel_stack_size / paging::PAGE_SIZE);

    return true;
}

void init_context(scheduler::process_t& process, const char* buffer, const std::string& file, const std::vector<std::string>& params){
    auto header = reinterpret_cast<const elf::elf_header*>(buffer);

    auto pages = scheduler::user_stack_size / paging::PAGE_SIZE;

    physical_pointer phys_ptr(process.physical_user_stack, pages);

    //1. Compute the size of the arguments on the stack

    //One pointer for each args
    size_t args_size = sizeof(size_t) * (1 + params.size());

    //Add the size of the default argument (+ terminating char)
    args_size += file.size() + 1;

    //Add the size of all the other arguments (+ terminating char)
    for(auto& param : params){
        args_size += param.size() + 1;
    }

    //Align the size on 16 bytes
    if(args_size % 16 != 0){
        args_size = ((args_size / 16) + 1) * 16;
    }

    //2. Modify the stack to add the args

    auto rsp = phys_ptr.get() + scheduler::user_stack_size - 8;
    auto arrays_start = scheduler::user_rsp - sizeof(size_t) * (1 + params.size());

    //Add pointer to each string
    size_t acc = 0;
    for(int64_t i = params.size() - 1; i >= 0; --i){
        auto& param = params[i];

        acc += param.size();

        *(reinterpret_cast<size_t*>(rsp)) = arrays_start - acc;
        rsp -= sizeof(size_t);

        ++acc;
    }

    //Add pointer to the default argument string
    *(reinterpret_cast<size_t*>(rsp)) = arrays_start - acc - file.size();
    rsp -= sizeof(size_t);

    //Add the strings of the arguments
    for(int64_t i = params.size() - 1; i >= 0; --i){
        auto& param = params[i];

        *(reinterpret_cast<char*>(rsp--)) = '\0';
        for(int64_t j = param.size() - 1; j >= 0; --j){
            *(reinterpret_cast<char*>(rsp--)) = param[j];
        }
    }

    //Add the the string of the default argument
    *(reinterpret_cast<char*>(rsp--)) = '\0';
    for(int64_t i = file.size() - 1; i >= 0; --i){
        *(reinterpret_cast<char*>(rsp--)) = file[i];
    }

    //3. Modify the stack to configure the context

    rsp = phys_ptr.get() + scheduler::user_stack_size - sizeof(interrupt::syscall_regs) - 8 - args_size;
    auto regs = reinterpret_cast<interrupt::syscall_regs*>(rsp);

    regs->rsp = scheduler::user_rsp - sizeof(interrupt::syscall_regs) - args_size; //Not sure about that
    regs->rip = header->e_entry;
    regs->cs = gdt::USER_CODE_SELECTOR + 3;
    regs->ds = gdt::USER_DATA_SELECTOR + 3;
    regs->rflags = 0x200;

    regs->rdi = 1 + params.size(); //argc
    regs->rsi = scheduler::user_rsp - params.size() * sizeof(size_t); //argv

    process.context = reinterpret_cast<interrupt::syscall_regs*>(scheduler::user_rsp - sizeof(interrupt::syscall_regs) - args_size);
}

void start() __attribute__((noreturn));
void start(){
    started = true;

    for(auto& process : pcb){
        if(process.state != scheduler::process_state::EMPTY){
            k_printf("pid: %u ppid: %u\n", process.process.pid, process.process.ppid);
        }
    }

    init_task_switch(current_pid);
}

} //end of anonymous namespace

void scheduler::init(){ //Create the idle task
    //Init all the semaphores
    for(auto& lock : run_queue_locks){
        lock.init();
    }

    //Create all the kernel tasks
    create_idle_task();
    create_init_task();
    create_gc_task();

    current_ticks = 0;

    //Run the init queue by default
    current_pid = 1;
    pcb[current_pid].rounds = TURNOVER;
    pcb[current_pid].state = scheduler::process_state::RUNNING;
}

int64_t scheduler::exec(const std::string& file, const std::vector<std::string>& params){
    //TODO Once shell removed, start will be called in init()
    if(!started){
        start();
        //Unreachable
    }

    auto content = disks::read_file(file);

    if(content.empty()){
        if(DEBUG_SCHEDULER){
            k_print_line("Not a file");
        }

        return -std::ERROR_NOT_EXISTS;
    }

    if(!elf::is_valid(content)){
        if(DEBUG_SCHEDULER){
            k_print_line("Not a valid file");
        }

        return -std::ERROR_NOT_EXECUTABLE;
    }

    auto buffer = content.c_str();

    auto& process = new_process();

    if(!create_paging(buffer, process)){
        if(DEBUG_SCHEDULER){
            k_print_line("Impossible to create paging");
        }

        return -std::ERROR_FAILED_EXECUTION;
    }

    process.brk_start = program_break;
    process.brk_end = program_break;

    init_context(process, buffer, file, params);

    queue_process(process.pid);

    return process.pid;
}

void scheduler::sbrk(size_t inc){
    auto& process = pcb[current_pid].process;

    size_t size = (inc + paging::PAGE_SIZE - 1) & ~(paging::PAGE_SIZE - 1);
    size_t pages = size / paging::PAGE_SIZE;

    //Get some physical memory
    auto physical = physical_allocator::allocate(pages);

    if(!physical){
        return;
    }

    auto virtual_start = process.brk_start;

    if(DEBUG_SCHEDULER){
        k_printf("Map(p%u) virtual:%h into phys: %h\n", process.pid, virtual_start, physical);
    }

    //Map the memory inside the process memory space
    if(!paging::user_map_pages(process, virtual_start, physical, pages)){
        k_print_line("impossible");
        physical_allocator::free(physical, pages);
        return;
    }

    process.segments.push_back({physical, pages});

    process.brk_end += size;
}

void scheduler::await_termination(pid_t pid){
    while(true){
        bool found = false;
        for(auto& process : pcb){
            if(process.process.ppid == current_pid && process.process.pid == pid){
                if(process.state == process_state::KILLED){
                    return;
                }

                found = true;
            }
        }

        if(!found){
            return;
        }

        pcb[current_pid].state = process_state::WAITING;
        reschedule();
    }
}

void scheduler::kill_current_process(){
    if(DEBUG_SCHEDULER){
        k_printf("Kill %u\n", current_pid);
    }

    //Notify parent if waiting
    auto ppid = pcb[current_pid].process.ppid;
    for(auto& process : pcb){
        if(process.process.pid == ppid && process.state == process_state::WAITING){
            unblock_process(process.process.pid);
        }
    }

    //The GC thread will clean up eventually
    unblock_process(gc_pid);

    pcb[current_pid].state = scheduler::process_state::KILLED;

    //Run another process
    reschedule();
}

void scheduler::tick(){
    if(!started){
        return;
    }

    ++current_ticks;

    for(auto& process : pcb){
        if(process.state == process_state::SLEEPING){
            --process.sleep_timeout;

            if(process.sleep_timeout == 0){
                process.state = process_state::READY;
            }
        }
    }

    if(current_ticks % QUANTUM_SIZE == 0){
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
    }

    //At this point we just have to return to the current process
}

void scheduler::reschedule(){
    thor_assert(started, "No interest in rescheduling before start");

    auto& process = pcb[current_pid];

    //The process just got blocked or put to sleep, choose another one
    if(process.state != process_state::RUNNING){
        auto index = select_next_process();

        switch_to_process(index);
    }

    //At this point we just have to return to the current process
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

    if(DEBUG_SCHEDULER){
        k_printf("Block process %u\n", pid);
    }

    pcb[pid].state = process_state::BLOCKED;

    reschedule();
}

void scheduler::unblock_process(pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pcb[pid].state == process_state::BLOCKED || pcb[pid].state == process_state::WAITING, "Can only unblock BLOCKED/WAITING processes");

    if(DEBUG_SCHEDULER){
        k_printf("Unblock process %u\n", pid);
    }

    pcb[pid].state = process_state::READY;
}

void scheduler::sleep_ms(pid_t pid, size_t time){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pcb[pid].state == process_state::RUNNING, "Only RUNNING processes can sleep");

    if(DEBUG_SCHEDULER){
        k_printf("Put %u to sleep\n", pid);
    }

    pcb[pid].state = process_state::SLEEPING;
    pcb[pid].sleep_timeout = time;

    reschedule();
}

size_t scheduler::register_new_handle(const std::string& path){
    pcb[current_pid].handles.push_back(path);

    return pcb[current_pid].handles.size() - 1;
}

bool scheduler::has_handle(size_t fd){
    return pcb[current_pid].handles.size() - 1 <= fd;
}

const std::string& scheduler::get_handle(size_t fd){
    return pcb[current_pid].handles[fd];
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
