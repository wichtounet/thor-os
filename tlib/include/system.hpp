//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_SYSTEM_HPP
#define USER_SYSTEM_HPP

#include <types.hpp>
#include <expected.hpp>
#include <vector.hpp>
#include <string.hpp>
#include <datetime.hpp>

void exit(size_t return_code) __attribute__((noreturn));

std::expected<size_t> exec(const char* executable, const std::vector<std::string>& params = {});
std::expected<size_t> exec_and_wait(const char* executable, const std::vector<std::string>& params = {});

void await_termination(size_t pid);

void sleep_ms(size_t ms);

datetime local_date();

void reboot();
void shutdown();

#endif
