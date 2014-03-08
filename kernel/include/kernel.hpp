//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef KERNEL_HPP
#define KERNEL_HPP

extern "C" {

void kernel_main() __attribute__((noreturn));

}

void suspend_boot() __attribute__((noreturn));
void suspend_kernel() __attribute__((noreturn));

#endif
