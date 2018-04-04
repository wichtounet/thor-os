//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef KERNEL_HPP
#define KERNEL_HPP

void suspend_boot() __attribute__((noreturn));
void suspend_kernel() __attribute__((noreturn));

#endif
