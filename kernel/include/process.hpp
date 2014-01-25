//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PROCESS_H
#define PROCESS_H

#include "stl/types.hpp"
#include "stl/vector.hpp"

namespace scheduler {

struct segment_t {
    size_t physical;
    size_t size;
};

struct process_t {
    size_t physical_cr3;
    size_t paging_size;

    size_t physical_user_stack;
    size_t physical_kernel_stack;

    std::vector<segment_t> segments;
    std::vector<size_t> physical_paging;
};

} //end of namespace scheduler

#endif
