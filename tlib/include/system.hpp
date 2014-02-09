//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_SYSTEM_HPP
#define USER_SYSTEM_HPP

#include <types.hpp>
#include <expected.hpp>

void exit(size_t return_code) __attribute__((noreturn));

int64_t exec(const char* executable);

void await_termination(size_t pid);

int64_t exec_and_wait(const char* executable);

void sleep_ms(size_t ms);

#endif
