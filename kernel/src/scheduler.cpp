//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <array.hpp>
#include <vector.hpp>
#include <optional.hpp>
#include <string.hpp>
#include <lock_guard.hpp>

#include <tlib/errors.hpp>
#include <tlib/elf.hpp>

#include "conc/int_lock.hpp"

#include "scheduler.hpp"
#include "paging.hpp"
#include "assert.hpp"
#include "gdt.hpp"
#include "stdio.hpp"
#include "disks.hpp"
#include "print.hpp"
#include "physical_allocator.hpp"
#include "virtual_allocator.hpp"
#include "physical_pointer.hpp"
#include "kernel_utils.hpp"
#include "logging.hpp"
#include "timer.hpp"
#include "kernel.hpp"

#include "fs/procfs.hpp"

//Provided by task_switch.s
extern "C" {
extern void task_switch(size_t current, size_t next);
extern void init_task_switch(size_t current) __attribute__((noreturn));
}

#ifdef THOR_CONFIG_SCHED_VERBOSE
#define verbose_logf(...) logging::logf(__VA_ARGS__)
#else
#define verbose_logf(...)
#endif

namespace {

constexpr const size_t STACK_ALIGNMENT = 16;     ///< In bytes
constexpr const size_t ROUND_ROBIN_QUANTUM = 25; ///< In milliseconds

//The Process Control Block
using pcb_t = std::array<scheduler::process_control_t, scheduler::MAX_PROCESS>;

pcb_t pcb;

//Define one run queue for each priority level
std::array<std::vector<scheduler::pid_t>, scheduler::PRIORITY_LEVELS> run_queues;

volatile bool started = false;

volatile size_t rr_quantum = 0;

volatile size_t current_pid;
volatile size_t next_pid = 0;

size_t gc_pid = 0;
size_t idle_pid = 0;
size_t init_pid = 0;
size_t post_init_pid = 0;

std::vector<scheduler::pid_t>& run_queue(size_t priority){
    return run_queues[priority - scheduler::MIN_PRIORITY];
}

void idle_task(){
    while(true){
        asm volatile("hlt");

        //If we go out of 'hlt', there have been an IRQ
        //There is probably someone ready, let's yield
        scheduler::yield();
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
                auto prev_pid = desc.pid;

                logging::logf(logging::log_level::DEBUG, "scheduler: Clean process %u\n", prev_pid);

                // 0. Notify parent if still waiting
                auto ppid = desc.ppid;
                for(auto& parent_process : pcb){
                    if(parent_process.process.pid == ppid && parent_process.state == scheduler::process_state::WAITING){
                        scheduler::unblock_process(parent_process.process.pid);
                    }
                }

                // 1. Release physical memory of PML4T (if not system task)

                if(!desc.system){
                    physical_allocator::free(desc.physical_cr3, 1);
                }

                // 2. Release physical stacks (if dynamically allocated)

                if(desc.physical_kernel_stack){
                    physical_allocator::free(desc.physical_kernel_stack, scheduler::kernel_stack_size / paging::PAGE_SIZE);
                }

                if(desc.physical_user_stack){
                    physical_allocator::free(desc.physical_user_stack, scheduler::user_stack_size / paging::PAGE_SIZE);
                }

                // kernel processes can either use dynamic memory or static memory

                if(desc.system){
                    if(reinterpret_cast<size_t>(desc.user_stack) >= paging::virtual_paging_start + (paging::physical_memory_pages * paging::PAGE_SIZE)){
                        delete[] desc.user_stack;
                    }

                    if(reinterpret_cast<size_t>(desc.kernel_stack) >= paging::virtual_paging_start + (paging::physical_memory_pages * paging::PAGE_SIZE)){
                        delete[] desc.kernel_stack;
                    }
                }

                // 3. Release segment's physical memory

                for(auto& segment : desc.segments){
                    physical_allocator::free(segment.physical, segment.size / paging::PAGE_SIZE);
                }
                desc.segments.clear();

                // 4. Release virtual kernel stack

                if(desc.virtual_kernel_stack){
                    virtual_allocator::free(desc.virtual_kernel_stack, scheduler::kernel_stack_size / paging::PAGE_SIZE);
                    paging::unmap_pages(desc.virtual_kernel_stack, scheduler::kernel_stack_size / paging::PAGE_SIZE);
                }

                // 5. Remove process from run queue

                {
                    // Move only write to the list with a lock
                    direct_int_lock queue_lock;

                    for(size_t index = 0; index < run_queue(desc.priority).size(); ++index){
                        if(run_queue(desc.priority)[index] == desc.pid){
                            run_queue(desc.priority).erase(index);
                            break;
                        }
                    }
                }

                // 6. Clean process

                desc.pid = 0;
                desc.ppid = 0;
                desc.system = false;
                desc.physical_cr3 = 0;
                desc.physical_user_stack = 0;
                desc.physical_kernel_stack = 0;
                desc.virtual_kernel_stack = 0;
                desc.paging_size = 0;
                desc.context = nullptr;
                desc.brk_start = desc.brk_end = 0;

                // 7. Clean file handles
                //TODO If not empty, probably something should be done
                process.handles.clear();

                // 8. Release the PCB slot
                process.state = scheduler::process_state::EMPTY;

                logging::logf(logging::log_level::DEBUG, "scheduler: Process %u cleaned\n", prev_pid);
            }
        }
    }
}

std::vector<void(*)()> init_tasks;

void post_init_task(){
    logging::logf(logging::log_level::DEBUG, "scheduler: post_init_task (pid:%u) started %u tasks\n", scheduler::get_pid(), init_tasks.size());

    for(auto func : init_tasks){
        func();
    }

    logging::logf(logging::log_level::DEBUG, "scheduler: post_init_task finished %u tasks\n", init_tasks.size());

    scheduler::kill_current_process();
}

//TODO tsh should be configured somewhere
void init_task(){
    logging::logf(logging::log_level::DEBUG, "scheduler: init_task started (pid:%d)\n", scheduler::get_pid());

    std::vector<std::string> params;

    while(true){
        auto pid = scheduler::exec("/bin/tsh", params);

        if(!pid){
            logging::logf(logging::log_level::DEBUG, "scheduler: failed to run the shell: %s\n", std::error_message(pid.error()));
            return;
        }

        scheduler::await_termination(*pid);

        logging::log(logging::log_level::DEBUG, "scheduler: shell exited, run new one\n");
    }
}

#define likely(x)    __builtin_expect (!!(x), 1)
#define unlikely(x)  __builtin_expect (!!(x), 0)

scheduler::pid_t get_free_pid(){
    // Normally, the next pid should be empty
    if(likely(pcb[next_pid].state == scheduler::process_state::EMPTY)){
        auto pid = next_pid;
        next_pid = (next_pid + 1) % scheduler::MAX_PROCESS;
        return pid;
    }

    // If the next pid is not available, iterate until one is empty

    auto pid = (next_pid + 1) % scheduler::MAX_PROCESS;
    size_t i = 0;

    while(pcb[pid].state != scheduler::process_state::EMPTY && i < scheduler::MAX_PROCESS){
        pid = (pid + 1) % scheduler::MAX_PROCESS;
        ++i;
    }

    // Make sure there was one free
    if(unlikely(i == scheduler::MAX_PROCESS)){
        logging::logf(logging::log_level::ERROR, "scheduler: Ran out of process\n");
        k_print_line("Ran out of processes");
        suspend_kernel();
    }

    // Set the next pid
    next_pid = (pid + 1) % scheduler::MAX_PROCESS;

    return pid;
}

scheduler::process_t& new_process(){
    auto pid = get_free_pid();

    auto& process = pcb[pid];

    process.process.system = false;
    process.process.pid = pid;
    process.process.ppid = current_pid;
    process.process.priority = scheduler::DEFAULT_PRIORITY;
    process.state = scheduler::process_state::NEW;
    process.process.tty = pcb[current_pid].process.tty;

    process.process.brk_start = 0;
    process.process.brk_end = 0;

    process.process.wait.pid = pid;
    process.process.wait.next = nullptr;

    // By default, a process is working in root
    process.working_directory = path("/");

    return process.process;
}

void queue_process(scheduler::pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");

    auto& process = pcb[pid];

    thor_assert(process.process.priority <= scheduler::MAX_PRIORITY, "Invalid priority");
    thor_assert(process.process.priority >= scheduler::MIN_PRIORITY, "Invalid priority");

    process.state = scheduler::process_state::READY;

    // Move only write to the list with a lock
    direct_int_lock queue_lock;
    run_queue(process.process.priority).push_back(pid);
}

void create_idle_task(){
    auto& idle_process = scheduler::create_kernel_task("idle", new char[scheduler::user_stack_size], new char[scheduler::kernel_stack_size], &idle_task);

    idle_process.ppid = 0;
    idle_process.priority = scheduler::MIN_PRIORITY;

    scheduler::queue_system_process(idle_process.pid);

    idle_pid = idle_process.pid;

    logging::logf(logging::log_level::DEBUG, "scheduler: idle_task %u \n", idle_pid);
}

void create_init_tasks(){
    for(size_t i = 0; i < stdio::terminals_count(); ++i){
        auto& init_process = scheduler::create_kernel_task("init", new char[scheduler::user_stack_size], new char[scheduler::kernel_stack_size], &init_task);

        init_process.ppid = 0;
        init_process.priority = scheduler::MIN_PRIORITY + 1;

        init_process.tty = i;

        auto pid = init_process.pid;
        if(i == 0){
            init_pid = pid;
        }

        auto tty = "/dev/tty" + std::to_string(i);

        // Create the 0,1,2 file descriptors
        pcb[pid].handles.emplace_back(tty);
        pcb[pid].handles.emplace_back(tty);
        pcb[pid].handles.emplace_back(tty);

        scheduler::queue_system_process(pid);

        logging::logf(logging::log_level::DEBUG, "scheduler: init_task %u tty:%u fd:%s\n", pid, init_process.tty, tty.c_str());
    }
}

void create_gc_task(){
    auto& gc_process = scheduler::create_kernel_task("gc", new char[scheduler::user_stack_size], new char[scheduler::kernel_stack_size], &gc_task);

    gc_process.ppid = 0;
    gc_process.priority = scheduler::MIN_PRIORITY + 1;

    scheduler::queue_system_process(gc_process.pid);

    gc_pid = gc_process.pid;

    logging::logf(logging::log_level::DEBUG, "scheduler: gc_task %u \n", gc_pid);
}

void create_post_init_task(){
    auto& post_init_process = scheduler::create_kernel_task("post_init", new char[scheduler::user_stack_size], new char[scheduler::kernel_stack_size], &post_init_task);

    post_init_process.ppid = 0;
    post_init_process.priority = scheduler::MAX_PRIORITY;

    post_init_pid = post_init_process.pid;

    scheduler::queue_system_process(post_init_pid);

    logging::logf(logging::log_level::DEBUG, "scheduler: post_init_task %u \n", post_init_pid);
}

void switch_to_process_with_lock(size_t new_pid){
    if (pcb[current_pid].process.system) {
        verbose_logf(logging::log_level::DEBUG, "scheduler: Switch from %u (s:%u) to %u (rip:%u)\n",
                     current_pid, static_cast<size_t>(pcb[current_pid].state), new_pid, pcb[current_pid].process.context->rip);
    } else {
        verbose_logf(logging::log_level::DEBUG, "scheduler: Switch from %u (s:%u) to %u\n",
                     current_pid, static_cast<size_t>(pcb[current_pid].state), new_pid);
    }

    auto old_pid = current_pid;

    // It is possible that preemption occured and that the process is already
    // running, in which case, there is no need to do anything
    if(old_pid == new_pid){
        return;
    }

    current_pid = new_pid;

    auto& process = pcb[new_pid];
    process.state = scheduler::process_state::RUNNING;

    gdt::tss().rsp0_low = process.process.kernel_rsp & 0xFFFFFFFF;
    gdt::tss().rsp0_high = process.process.kernel_rsp >> 32;

    task_switch(old_pid, current_pid);
}

void switch_to_process(size_t new_pid){
    // This should never be interrupted
    direct_int_lock l;

    switch_to_process_with_lock(new_pid);
}

/*!
 * \brief Select the next process to run.
 *
 * This function assume that an int_lock is already owned.
 */
size_t select_next_process_with_lock(){
    auto current_priority = pcb[current_pid].process.priority;

    //1. Run a process of higher priority, if any
    for(size_t p = scheduler::MAX_PRIORITY; p > current_priority; --p){
        for(auto pid : run_queue(p)){
            if(pcb[pid].state == scheduler::process_state::READY || pcb[pid].state == scheduler::process_state::RUNNING){
                return pid;
            }
        }
    }

    //2. Run the next process of the same priority

    {
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

            if(pcb[pid].state == scheduler::process_state::READY || pcb[pid].state == scheduler::process_state::RUNNING){
                return pid;
            }
        }
    }

    thor_assert(current_priority > 0, "The idle task should always be ready");

    //3. Run a process of lower priority

    for(size_t p = current_priority - 1; p >= scheduler::MIN_PRIORITY; --p){
        for(auto pid : run_queue(p)){
            if(pcb[pid].state == scheduler::process_state::READY || pcb[pid].state == scheduler::process_state::RUNNING){
                return pid;
            }
        }
    }

    thor_unreachable("No process is READY");
}

size_t select_next_process(){
    // Cannot be interrupted
    direct_int_lock lock;

    return select_next_process_with_lock();
}

bool allocate_user_memory(scheduler::process_t& process, size_t address, size_t size, size_t& ref){
    //1. Calculate some stuff
    auto first_page = paging::page_align(address);
    auto left_padding = address - first_page;

    auto bytes = left_padding + size;
    auto pages = paging::pages(bytes);

    //2. Get enough physical memory
    auto physical_memory = physical_allocator::allocate(pages);

    if(!physical_memory){
        k_print_line("Cannot allocate physical memory, probably out of memory");
        return false;
    }

    //3. Find a start of a page inside the physical memory

    auto aligned_physical_memory = paging::page_aligned(physical_memory) ? physical_memory :
            (physical_memory / paging::PAGE_SIZE + 1) * paging::PAGE_SIZE;

    logging::logf(logging::log_level::DEBUG, "scheduler: Map(p%u) virtual:%h into phys: %h\n", process.pid, first_page, aligned_physical_memory);

    //4. Map physical allocated memory to the necessary virtual memory
    if(!paging::user_map_pages(process, first_page, aligned_physical_memory, pages)){
        logging::log(logging::log_level::DEBUG, "Impossible to map in user space\n");

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

    logging::logf(logging::log_level::DEBUG, "scheduler: Process %u cr3:%h\n", process.pid, process.physical_cr3);

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

            logging::logf(logging::log_level::DEBUG, "scheduler: Copy to physical:%h\n", segment.physical);

            auto memory_start = phys_ptr.get() + left_padding;

            //In the case of the BSS segment, the segment must be
            //filled with zero
            if(p_header.p_filesize != p_header.p_memsz){
                std::copy_n(buffer + p_header.p_offset, p_header.p_filesize, reinterpret_cast<char*>(memory_start));
                std::fill_n(reinterpret_cast<char*>(memory_start + p_header.p_filesize), p_header.p_memsz - p_header.p_filesize, 0);
            } else {
                std::copy_n(buffer + p_header.p_offset, p_header.p_memsz, reinterpret_cast<char*>(memory_start));
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
    regs->rbp = 0;
    regs->rip = header->e_entry;
    regs->cs = gdt::USER_CODE_SELECTOR + 3;
    regs->ds = gdt::USER_DATA_SELECTOR + 3;
    regs->rflags = 0x200;

    regs->rdi = 1 + params.size(); //argc
    regs->rsi = scheduler::user_rsp - params.size() * sizeof(size_t); //argv

    process.context = reinterpret_cast<interrupt::syscall_regs*>(scheduler::user_rsp - sizeof(interrupt::syscall_regs) - args_size);
}

} //end of anonymous namespace

//Provided for task_switch.s

extern "C" {

uint64_t get_context_address(size_t pid){
    return reinterpret_cast<size_t>(&pcb[pid].process.context);
}

uint64_t get_process_cr3(size_t pid){
    return reinterpret_cast<uint64_t>(pcb[pid].process.physical_cr3);
}

} //end of extern "C"

void scheduler::init(){
    //Create all the kernel tasks
    create_idle_task();
    create_init_tasks();
    create_gc_task();
    create_post_init_task();

    procfs::set_pcb(pcb.data());

    logging::logf(logging::log_level::TRACE, "scheduler: initialized (PCB size:%m pcb_entry:%m process: %m)\n", sizeof(pcb_t), sizeof(process_control_t), sizeof(process_t));
}

void scheduler::start(){
    logging::log(logging::log_level::TRACE, "scheduler: starting\n");

    // TODO The current_pid should be set dynamically to the task in the list
    // with highest priority

    //Run the post init task by default (maximum priority)
    current_pid = post_init_pid;
    pcb[current_pid].state = scheduler::process_state::RUNNING;

    started = true;

    init_task_switch(current_pid);
}

bool scheduler::is_started(){
    return started;
}

std::expected<scheduler::pid_t> scheduler::exec(const std::string& file, const std::vector<std::string>& params){
    logging::log(logging::log_level::TRACE, "scheduler:exec: read_file start\n");

    std::string content;
    auto result = vfs::direct_read(path(file), content);
    if(!result){
        logging::logf(logging::log_level::DEBUG, "scheduler: direct_read error: %s\n", std::error_message(result.error()));

        return std::make_unexpected<pid_t, size_t>(result.error());
    }

    logging::log(logging::log_level::TRACE, "scheduler:exec: read_file end\n");

    if(content.empty()){
        logging::log(logging::log_level::DEBUG, "scheduler:exec: Not a file\n");

        return std::make_unexpected<pid_t>(std::ERROR_NOT_EXISTS);
    }

    auto buffer = content.c_str();

    if(!elf::is_valid(buffer)){
        logging::log(logging::log_level::DEBUG, "scheduler:exec: Not a valid file\n");

        return std::make_unexpected<pid_t>(std::ERROR_NOT_EXECUTABLE);
    }

    auto& process = new_process();

    process.name = file;

    if(!create_paging(buffer, process)){
        logging::log(logging::log_level::DEBUG, "scheduler:exec: Impossible to create paging\n");

        return std::make_unexpected<pid_t>(std::ERROR_FAILED_EXECUTION);
    }

    process.brk_start = program_break;
    process.brk_end = program_break;

    init_context(process, buffer, file, params);

    pcb[process.pid].working_directory = pcb[current_pid].working_directory;

    // Inherit standard file descriptors from the parent
    pcb[process.pid].handles.emplace_back(pcb[current_pid].handles[0]);
    pcb[process.pid].handles.emplace_back(pcb[current_pid].handles[1]);
    pcb[process.pid].handles.emplace_back(pcb[current_pid].handles[2]);

    logging::logf(logging::log_level::DEBUG, "scheduler: Exec process pid=%u, ppid=%u\n", process.pid, process.ppid);

    queue_process(process.pid);

    return process.pid;
}

void scheduler::sbrk(size_t inc){
    auto& process = pcb[current_pid].process;

    size_t size = (inc + paging::PAGE_SIZE - 1) & ~(paging::PAGE_SIZE - 1);
    size_t pages = size / paging::PAGE_SIZE;

    logging::logf(logging::log_level::DEBUG, "sbrk: Add %u pages to process %u heap\n", pages, process.pid);

    //Get some physical memory
    auto physical = physical_allocator::allocate(pages);

    if(!physical){
        logging::logf(logging::log_level::DEBUG, "sbrk: Impossible to allocate %u pages for process %u\n", pages, process.pid);
        return;
    }

    auto virtual_start = process.brk_end;

    logging::logf(logging::log_level::DEBUG, "sbrk: Map(p%u) virtual:%h into phys: %h\n", process.pid, virtual_start, physical);

    //Map the memory inside the process memory space
    if(!paging::user_map_pages(process, virtual_start, physical, pages)){
        physical_allocator::free(physical, pages);
        return;
    }

    process.segments.push_back({physical, pages});

    process.brk_end += size;
}

void scheduler::await_termination(pid_t pid){
    while(true){
        {
            direct_int_lock lock;

            bool found = false;
            for(auto& process : pcb){
                if(process.process.ppid == current_pid && process.process.pid == pid){
                    if(process.state == process_state::KILLED || process.state == process_state::EMPTY){
                        return;
                    }

                    found = true;
                    break;
                }
            }

            // The process may have already been cleaned, we can simply return
            if(!found){
                return;
            }

            logging::logf(logging::log_level::DEBUG, "scheduler: Process %u waits for %u\n", current_pid, pid);

            pcb[current_pid].state = process_state::WAITING;
        }

        // Reschedule is out of the critical section
        reschedule();
    }
}

void scheduler::kill_current_process(){
    logging::logf(logging::log_level::DEBUG, "scheduler: Kill %u\n", current_pid);

    {
        direct_int_lock lock;

        // The process is now considered killed
        pcb[current_pid].state = scheduler::process_state::KILLED;

        //Notify parent if waiting
        auto ppid = pcb[current_pid].process.ppid;
        for(auto& process : pcb){
            if(process.process.pid == ppid && process.state == process_state::WAITING){
                unblock_process(process.process.pid);
            }
        }

        //The GC thread will clean the resources eventually
        if(pcb[gc_pid].state == process_state::BLOCKED){
            unblock_process(gc_pid);
        }
    }

    //Run another process
    reschedule();

    thor_unreachable("A killed process has been run!");
}

void scheduler::tick(){
    if(!started){
        return;
    }

    // Update sleep timeouts
    for(auto& process : pcb){
        if(process.state == process_state::SLEEPING || process.state == process_state::BLOCKED_TIMEOUT){
            --process.sleep_timeout;

            if(process.sleep_timeout == 0){
                verbose_logf(logging::log_level::TRACE, "scheduler: Process %u finished sleeping, is ready\n", process.process.pid);
                process.state = process_state::READY;
            }
        }
    }

    auto& process = pcb[current_pid];

    if(process.rounds == rr_quantum){
        process.rounds = 0;

        auto previous_state = process.state;

        // Change to Ready if it was not blocked
        // If it was blocked, we still prempt and it will end up in reschedule
        // later but with a full time quanta
        if(previous_state != process_state::BLOCKED && previous_state != process_state::BLOCKED_TIMEOUT){
            process.state = process_state::READY;
        }

        auto pid = select_next_process();

        //If it is the same, no need to go to the switching process
        if(pid == current_pid){
            process.state = previous_state;
            return;
        }

        verbose_logf(logging::log_level::DEBUG, "scheduler: Preempt %u (%d->%d) to %u\n", current_pid, previous_state, process.state, pid);

        switch_to_process(pid);
    } else {
        ++process.rounds;
    }

    //At this point we just have to return to the current process
}

void scheduler::yield(){
    thor_assert(started, "No interest in yielding before start");
    thor_assert(pcb[current_pid].state == process_state::RUNNING, "Can only yield() running processes");

    pcb[current_pid].state = process_state::READY;

    direct_int_lock lock;

    auto pid = select_next_process_with_lock();

    if(pid != current_pid){
        switch_to_process_with_lock(pid);
    } else {
        pcb[current_pid].state = process_state::RUNNING;
    }
}

void scheduler::reschedule(){
    thor_assert(started, "No interest in rescheduling before start");

    auto& process = pcb[current_pid];

    //The process just got blocked or put to sleep, choose another one
    if(process.state != process_state::RUNNING){
        direct_int_lock lock;

        auto index = select_next_process_with_lock();

        switch_to_process_with_lock(index);
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

scheduler::process_state scheduler::get_process_state(pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");

    return pcb[pid].state;
}

void scheduler::block_process_light(pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");

    verbose_logf(logging::log_level::DEBUG, "scheduler: Block process (light) %u\n", pid);

    pcb[pid].state = process_state::BLOCKED;
}

void scheduler::block_process_timeout_light(pid_t pid, size_t ms){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");

    verbose_logf(logging::log_level::DEBUG, "scheduler: Block process (light) %u with timeout %u\n", pid, ms);

    // Compute the amount of ticks to sleep
    auto sleep_ticks = ms * (timer::timer_frequency() / 1000);
    sleep_ticks = !sleep_ticks ? 1 : sleep_ticks;

    // Put the process to sleep
    pcb[pid].sleep_timeout = sleep_ticks;

    pcb[pid].state = process_state::BLOCKED_TIMEOUT;
}

void scheduler::block_process(pid_t pid){
    thor_assert(is_started(), "The scheduler is not started");
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pid != idle_pid, "No reason to block the idle task");

    verbose_logf(logging::log_level::DEBUG, "scheduler: Block process %u\n", pid);

    pcb[pid].state = process_state::BLOCKED;

    reschedule();
}

void scheduler::unblock_process(pid_t pid){
    verbose_logf(logging::log_level::DEBUG, "scheduler: Unblock process %u (%u)\n", pid, size_t(pcb[pid].state));

    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pid != idle_pid, "No reason to unblock the idle task");
    thor_assert(is_started(), "The scheduler is not started");
    thor_assert(pcb[pid].state == process_state::BLOCKED || pcb[pid].state == process_state::BLOCKED_TIMEOUT || pcb[pid].state == process_state::WAITING, "Can only unblock BLOCKED/WAITING processes");

    pcb[pid].state = process_state::READY;
}

void scheduler::unblock_process_hint(pid_t pid){
    verbose_logf(logging::log_level::DEBUG, "scheduler: Unblock process (hint) %u (%u)\n", pid, size_t(pcb[pid].state));

    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pid != idle_pid, "No reason to unblock the idle task");
    thor_assert(is_started(), "The scheduler is not started");

    auto state = pcb[pid].state;

    if(state != process_state::RUNNING){
        pcb[pid].state = process_state::READY;
    }
}

void scheduler::sleep_ms(size_t time){
    sleep_ms(current_pid, time);
}

void scheduler::sleep_ms(pid_t pid, size_t time){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");
    thor_assert(pcb[pid].state == process_state::RUNNING, "Only RUNNING processes can sleep");

    // Compute the amount of ticks to sleep
    auto sleep_ticks = time * (timer::timer_frequency() / 1000);
    sleep_ticks = !sleep_ticks ? 1 : sleep_ticks;

    logging::logf(logging::log_level::DEBUG, "scheduler: Put %u to sleep for %u ticks\n", pid, sleep_ticks);

    // Put the process to sleep
    pcb[pid].sleep_timeout = sleep_ticks;
    pcb[pid].state = process_state::SLEEPING;

    // Run another process
    reschedule();
}

size_t scheduler::register_new_handle(const path& p){
    pcb[current_pid].handles.push_back(p);

    return pcb[current_pid].handles.size();
}

void scheduler::release_handle(size_t fd){
    pcb[current_pid].handles[fd - 1].invalidate();
}

bool scheduler::has_handle(size_t fd){
    return fd > 0 && fd <= pcb[current_pid].handles.size() && pcb[current_pid].handles[fd - 1].is_valid();
}

const path& scheduler::get_handle(size_t fd){
    return pcb[current_pid].handles[fd - 1];
}

size_t scheduler::register_new_socket(network::socket_domain domain, network::socket_type type, network::socket_protocol protocol){
    auto id = pcb[current_pid].sockets.size() + 1;

    pcb[current_pid].sockets.emplace_back(id, domain, type, protocol, size_t(1), false);

    return id;
}

void scheduler::release_socket(size_t fd){
    pcb[current_pid].sockets[fd - 1].invalidate();
}

bool scheduler::has_socket(size_t fd){
    return fd > 0 && fd - 1 < pcb[current_pid].sockets.size() && pcb[current_pid].sockets[fd - 1].is_valid();
}

network::socket& scheduler::get_socket(size_t fd){
    return pcb[current_pid].sockets[fd - 1];
}

std::deque<network::socket>& scheduler::get_sockets(){
    return pcb[current_pid].sockets;
}

std::deque<network::socket>& scheduler::get_sockets(scheduler::pid_t pid){
    return pcb[pid].sockets;
}

const path& scheduler::get_working_directory(){
    return pcb[current_pid].working_directory;
}

void scheduler::set_working_directory(const path& directory){
    pcb[current_pid].working_directory = directory;
}

scheduler::process_t& scheduler::create_kernel_task(const char* name, char* user_stack, char* kernel_stack, void (*fun)()){
    auto& process = new_process();

    process.system = true;
    process.physical_cr3 = paging::get_physical_pml4t();
    process.paging_size = 0;
    process.name = name;

    // Directly uses memory of the executable
    process.physical_user_stack = 0;
    process.physical_kernel_stack = 0;
    process.virtual_kernel_stack = 0;

    process.user_stack = user_stack;
    process.user_stack = kernel_stack;

    auto rsp = &user_stack[scheduler::user_stack_size - STACK_ALIGNMENT];
    rsp -= sizeof(interrupt::syscall_regs);

    process.context = reinterpret_cast<interrupt::syscall_regs*>(rsp);

    process.context->rflags = 0x200;
    process.context->rip = reinterpret_cast<size_t>(fun);
    process.context->rsp = reinterpret_cast<size_t>(rsp);
    process.context->cs = gdt::LONG_SELECTOR;
    process.context->ds = gdt::DATA_SELECTOR;

    process.kernel_rsp = reinterpret_cast<size_t>(&kernel_stack[scheduler::kernel_stack_size - STACK_ALIGNMENT]);

    thor_assert((reinterpret_cast<size_t>(process.context) - sizeof(interrupt::syscall_regs)) % STACK_ALIGNMENT == 0, "Process context must be correctly aligned");
    thor_assert((process.context->rsp - sizeof(interrupt::syscall_regs)) % STACK_ALIGNMENT == 0, "Process user stack must be correctly aligned");
    thor_assert(process.kernel_rsp % STACK_ALIGNMENT == 0, "Process kernel stack must be correctly aligned");

    return process;
}

scheduler::process_t& scheduler::create_kernel_task_args(const char* name, char* user_stack, char* kernel_stack, void (*fun)(void*), void* data){
    auto& process = scheduler::create_kernel_task(name, user_stack, kernel_stack, reinterpret_cast<void(*)()>(fun));

    // rdi is the first register used for integers parameter passing
    process.context->rdi = reinterpret_cast<size_t>(data);

    return process;
}

void scheduler::queue_system_process(scheduler::pid_t pid){
    thor_assert(pid < scheduler::MAX_PROCESS, "pid out of bounds");

    auto& process = pcb[pid];

    thor_assert(process.process.priority <= scheduler::MAX_PRIORITY, "Invalid priority");
    thor_assert(process.process.priority >= scheduler::MIN_PRIORITY, "Invalid priority");

    process.state = scheduler::process_state::READY;

    run_queue(process.process.priority).push_back(pid);
}

void scheduler::queue_async_init_task(void (*fun)()){
    init_tasks.emplace_back(fun);
}

void scheduler::frequency_updated(uint64_t old_frequency, uint64_t new_frequency){
    // Cannot be interrupted during frequency update
    direct_int_lock lock;

    rr_quantum = ROUND_ROBIN_QUANTUM * (double(new_frequency) / double(1000));

    double ratio = old_frequency / double(new_frequency);

    for(auto& process : pcb){
        if(process.state == process_state::SLEEPING){
            process.sleep_timeout *= ratio;
        }
    }

    logging::logf(logging::log_level::DEBUG, "scheduler:: Frequency updated. New Round Robin quantum: %u\n", rr_quantum);
}

void scheduler::fault(){
    logging::logf(logging::log_level::DEBUG, "scheduler: Fault in %u kill it\n", current_pid);

    kill_current_process();
}
