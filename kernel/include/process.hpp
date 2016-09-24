//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PROCESS_H
#define PROCESS_H

#include <types.hpp>
#include <vector.hpp>

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

typedef size_t pid_t;

constexpr const pid_t INVALID_PID = 1024 * 1024 * 1024; //I'm pretty sure we won't violate this limit

enum class process_state : char {
    EMPTY = 0,
    NEW = 1,
    READY = 2,
    RUNNING = 3,
    BLOCKED = 4,
    SLEEPING= 5,
    WAITING = 6,
    KILLED = 7,
    BLOCKED_TIMEOUT = 8
};

struct segment_t {
    size_t physical;
    size_t size;
};

struct process_t {
    pid_t pid;  ///< The process id
    pid_t ppid; ///< The parent's process id

    bool system; ///< Indicates if the process is a system process

    size_t priority; ///< The priority of the process

    size_t tty; ///< The terminal the process is linked to

    size_t physical_cr3;
    size_t paging_size;

    size_t physical_user_stack;
    size_t physical_kernel_stack;
    size_t virtual_kernel_stack;

    size_t kernel_rsp;

    size_t brk_start;
    size_t brk_end;

    // Only for system kernels
    char* user_stack;
    char* kernel_stack;

    volatile interrupt::syscall_regs* context;

    wait_node wait;

    std::vector<segment_t> segments;

    std::string name;
};

constexpr const size_t program_base = 0x8000000000;
constexpr const size_t program_break = 0x9000000000;

constexpr const auto user_stack_size = 2 * paging::PAGE_SIZE;
constexpr const auto kernel_stack_size = 2 * paging::PAGE_SIZE;

constexpr const auto user_stack_start = program_base + 0x700000;
constexpr const auto user_rsp = user_stack_start + (user_stack_size - 8);

struct process_control_t {
    scheduler::process_t process;
    scheduler::process_state state;
    size_t rounds;
    size_t sleep_timeout;
    std::vector<path> handles;
    std::vector<network::socket> sockets;
    path working_directory;
};

} //end of namespace scheduler

#endif
