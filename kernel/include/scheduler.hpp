//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector.hpp>
#include <string.hpp>
#include <expected.hpp>

#include "process.hpp"
#include "net/socket.hpp"

#include "vfs/path.hpp"

namespace scheduler {

constexpr const size_t MAX_PROCESS = 128;

/*!
 * \brief Return the id of the current process
 */
pid_t get_pid();

/*!
 * \brief Return the process with the given ID
 */
scheduler::process_t& get_process(pid_t pid);

/*!
 * \brief Returns the state of the process with the given ID
 */
scheduler::process_state get_process_state(pid_t pid);

/*!
 * \brief Block the given process and immediately reschedule it
 */
void block_process(pid_t pid);

/*!
 * \brief Unblock a blocked process
 */
void unblock_process(pid_t pid);

/*!
 * \brief Unblock a process if it is a blocked
 */
void unblock_process_hint(pid_t pid);

/*!
 * \brief Init the scheduler
 */
void init();

/*!
 * \brief Start the scheduler and starts the first process
 */
void start() __attribute__((noreturn));

/*!
 * \brief Indicates if the scheduler is started or not
 */
bool is_started();

/*!
 * \brief Execute the given file in a new process
 */
std::expected<pid_t> exec(const std::string& path, const std::vector<std::string>& params);

/*!
 * \brief Kill the current process
 */
void kill_current_process() __attribute__((noreturn));

/*!
 * \brief Wait for the given process to terminate
 */
void await_termination(pid_t pid);

/*!
 * \brief Allocate more memory for the process
 * \param inc The amount of memory to add
 */
void sbrk(size_t inc);

/*!
 * \brief Let the scheduler know of a timer tick
 */
void tick();

/*!
 * \brief Let another process run.
 *
 * This may change the state of the current process state
 */
void yield();

/*!
 * \brief Reschedule to another process, if the current process is not running
 *
 * This will not change the state of the process!
 */
void reschedule();

/*!
 * \brief Indicates a fault in the current process
 */
void fault();

/*!
 * \brief Make the current process sleep for the given amount of milliseconds
 * \param time The number of milliseconds to wait
 */
void sleep_ms(size_t time);

/*!
 * \brief Make the given process sleep for the given amount of milliseconds
 * \param time The number of milliseconds to wait
 */
void sleep_ms(pid_t pid, size_t time);

/*!
 * \brief Register a new handle (file descriptor) for the current process
 */
size_t register_new_handle(const path& p);

/*!
 * \brief Get the path of the given file descriptor
 */
const path& get_handle(size_t fd);

/*!
 * \brief Indicates if the current process has the given file descriptor
 */
bool has_handle(size_t fd);

/*!
 * \brief Release the given handle from the current process
 */
void release_handle(size_t fd);

/*!
 * \brief Register a new socket for the current process
 * \param domain The socket domain
 * \param type The socket type
 * \param protocol The socket protocol
 */
size_t register_new_socket(network::socket_domain domain, network::socket_type type, network::socket_protocol protocol);

/*!
 * \brief Get the socket of the given file descriptor
 */
network::socket& get_socket(size_t fd);

/*!
 * \brief Indicates if the current process has the given socket
 */
bool has_socket(size_t fd);

/*!
 * \brief Release the given socket from the current process
 */
void release_socket(size_t fd);

/*!
 * \brief Returns the sockets of the current process
 */
std::deque<network::socket>& get_sockets();

/*!
 * \brief Returns the sockets of the given process
 */
std::deque<network::socket>& get_sockets(pid_t pid);

/*!
 * \brief Returns the working directory of the current process
 */
const path& get_working_directory();

/*!
 * \brief Changes the working directory of the current process
 */
void set_working_directory(const path& directory);

/*!
 * \brief Block the given process, but do not reschedule
 */
void block_process_light(pid_t pid);

/*!
 * \brief Block the given process, with a possible timeout, but do not reschedule
 */
void block_process_timeout_light(pid_t pid, size_t ms);

/*!
 * \brief Creat a kernel process
 * \param name The name of the process
 * \param user_stack Pointer to the user stack
 * \param kernel_stack Pointer to the kernel stack
 * \param fun The function to execute
 */
process_t& create_kernel_task(const char* name, char* user_stack, char* kernel_stack, void (*fun)());

/*!
 * \brief Creat a kernel process with some data
 * \param name The name of the process
 * \param user_stack Pointer to the user stack
 * \param kernel_stack Pointer to the kernel stack
 * \param fun The function to execute
 * \param data The data to pass to the function
 */
process_t& create_kernel_task_args(const char* name, char* user_stack, char* kernel_stack, void (*fun)(void*), void* data);

/*!
 * \brief Queue a created system process
 */
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
