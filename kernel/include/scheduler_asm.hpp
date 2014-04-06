//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SCHEDULER_ASM_H
#define SCHEDULER_ASM_H

extern "C" {

uint64_t get_context_address(size_t pid);
uint64_t get_process_cr3(size_t pid);

} //end of extern "C"

#endif
