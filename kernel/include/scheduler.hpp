//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector.hpp>
#include <string.hpp>

#include "process.hpp"

namespace scheduler {

constexpr const size_t MAX_PROCESS = 128;

void init();
void start() __attribute__((noreturn));

bool is_started();

pid_t get_pid();
scheduler::process_t& get_process(pid_t pid);

void block_process(pid_t pid);
void unblock_process(pid_t pid);

int64_t exec(const std::string& path, const std::vector<std::string>& params);

void kill_current_process();
void await_termination(pid_t pid);
void sbrk(size_t inc);

void tick();

void sleep_ms(pid_t pid, size_t time);

size_t register_new_handle(const std::vector<std::string>& path);
const std::vector<std::string>& get_handle(size_t fd);
bool has_handle(size_t fd);
void release_handle(size_t fd);

const std::vector<std::string>& get_working_directory();
void set_working_directory(const std::vector<std::string>& directory);

} //end of namespace scheduler

#endif
