//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GDT_H
#define GDT_H

#include "gdt_types.hpp"

namespace gdt {

void flush_tss();

task_state_segment_t& tss();

} //end of namespace gdt

#endif
