//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector.hpp>
#include <string.hpp>

#include "process.hpp"

#include "vfs/path.hpp"

namespace scheduler {

constexpr const size_t MAX_PROCESS = 128;

constexpr const pid_t INVALID_PID = 1024 * 1024 * 1024; //I'm pretty sure we won't violate this limit

pid_t get_pid();
scheduler::process_t& get_process(pid_t pid);
scheduler::process_state get_process_state(pid_t pid);

void block_process(pid_t pid);
void unblock_process(pid_t pid);
void unblock_process_hint(pid_t pid);

void init();
void start() __attribute__((noreturn));
bool is_started();

int64_t exec(const std::string& path, const std::vector<std::string>& params);

void kill_current_process();
void await_termination(pid_t pid);
void sbrk(size_t inc);

void tick();
void reschedule();

/*!
 * \brief Indicates a fault in the current process
 */
void fault();

void sleep_ms(size_t time);
void sleep_ms(pid_t pid, size_t time);

size_t register_new_handle(const path& p);
const path& get_handle(size_t fd);
bool has_handle(size_t fd);
void release_handle(size_t fd);

const path& get_working_directory();
void set_working_directory(const path& directory);

void block_process_light(pid_t pid);
void block_process_timeout_light(pid_t pid, size_t ms);

process_t& create_kernel_task(const char* name, char* user_stack, char* kernel_stack, void (*fun)());
process_t& create_kernel_task_args(const char* name, char* user_stack, char* kernel_stack, void (*fun)(void*), void* data);
void queue_system_process(pid_t pid);

/*!
 * \brief Queue an initilization task that will be run after the
 * scheduler is started
 *
 * This must be used for drivers that needs scheduling to be started
 * or for drivers depending on others drivers asynchronously
 * started.
 */
void queue_async_init_task(void (*fun)());

/*!
 * \brief Lets the scheduler know that the timer frequency has been updated
 */
void frequency_updated(uint64_t old_frequency, uint64_t new_frequency);

} //end of namespace scheduler

#endif
