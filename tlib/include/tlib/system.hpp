//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef USER_SYSTEM_HPP
#define USER_SYSTEM_HPP

#include <types.hpp>
#include <expected.hpp>
#include <vector.hpp>
#include <string.hpp>

#include "tlib/datetime.hpp"
#include "tlib/config.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

void exit(size_t return_code) __attribute__((noreturn));

std::expected<size_t> exec(const char* executable, const std::vector<std::string>& params = {});
std::expected<size_t> exec_and_wait(const char* executable, const std::vector<std::string>& params = {});

void await_termination(size_t pid);

void sleep_ms(size_t ms);

datetime local_date();

void reboot();
void shutdown();

uint64_t s_time();
uint64_t ms_time();

void alpha();

} // end of tlib namespace

#endif
