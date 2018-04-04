//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PROCESS_H
#define PROCESS_H

#include <types.hpp>
#include <vector.hpp>
#include <deque.hpp>

#include "paging.hpp"
#include "interrupts.hpp"
#include "conc/wait_list.hpp"

#include "vfs/path.hpp"

namespace network {

struct socket;

} // end of namespace network

namespace scheduler {

constexpr const size_t MAX_PRIORITY = 4;
constexpr const size_t MIN_PRIORITY = 1;
constexpr const size_t PRIORITY_LEVELS = MAX_PRIORITY - MIN_PRIORITY + 1;
constexpr const size_t DEFAULT_PRIORITY = 3;

using pid_t = size_t; ///< A process id

constexpr const pid_t INVALID_PID = 1024 * 1024 * 1024; //I'm pretty sure we won't violate this limit

enum class process_state : char {
    EMPTY = 0, ///< Not a process
    NEW = 1,  ///< A newly created process
    READY = 2, ///< A process ready to run
    RUNNING = 3, ///< A process currently executing
    BLOCKED = 4, ///< A blocked process
    SLEEPING= 5, ///< A sleeping process
    WAITING = 6, ///< A waiting process (for a child)
    KILLED = 7, ///< A killed process
    BLOCKED_TIMEOUT = 8 ///< A blocked, with timeout, process
};

/*!
 * \brief A physical segment of memory used by the process
 */
struct segment_t {
    size_t physical; ///< The physical start
    size_t size; ///< The size of allocated memory
};

struct process_t {
    pid_t pid;  ///< The process id
    pid_t ppid; ///< The parent's process id

    bool system; ///< Indicates if the process is a system process

    size_t priority; ///< The priority of the process

    size_t tty; ///< The terminal the process is linked to

    size_t physical_cr3; ///< The physical address of the CR3
    size_t paging_size; ///< The  size of the paging structure

    size_t physical_user_stack; ///< The physical address of the user stack
    size_t physical_kernel_stack; ///< The physical address of the kernel stack
    size_t virtual_kernel_stack; ///< The virtual address of the kernel stack

    size_t kernel_rsp; ///< The kernel stack pointer

    size_t brk_start; ///< The start of the brk section
    size_t brk_end; ///< The end of the brk section

    // Only for system kernels
    char* user_stack; ///< Pointer to the user stack
    char* kernel_stack; ///< Pointer to the kernel stack

    volatile interrupt::syscall_regs* context; ///< A pointer to the context

    wait_node wait; ///< The process's wait node

    std::vector<segment_t> segments; ///< The physical segments

    std::string name; ///< The name of the process
};

constexpr const size_t program_base = 0x8000000000; ///< The virtual address of a program start
constexpr const size_t program_break = 0x9000000000; ///< The virtual address of a program break start

constexpr const auto user_stack_size = 2 * paging::PAGE_SIZE; ///< The size of the user stack
constexpr const auto kernel_stack_size = 2 * paging::PAGE_SIZE; ///< The size of the kernel stack

constexpr const auto user_stack_start = program_base + 0x700000; ///< The virtual address of a program user stack
constexpr const auto user_rsp = user_stack_start + (user_stack_size - 8); ///< The initial program stack pointer

/*!
 * \brief An entry in the Process Control Block
 */
struct process_control_t {
    scheduler::process_t process; ///< The process itself
    scheduler::process_state state; ///< The state of the process
    size_t rounds; ///< The number of rounds remaining
    size_t sleep_timeout; ///< The sleep timeout (in ticks)
    std::vector<path> handles; ///< The file handles
    std::deque<network::socket> sockets; ///< The socket handles
    path working_directory; ///< The current working directory
};

} //end of namespace scheduler

#endif
