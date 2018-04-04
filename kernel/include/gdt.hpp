//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef GDT_H
#define GDT_H

#include "gdt_types.hpp"

namespace gdt {

void flush_tss();

task_state_segment_t& tss();

} //end of namespace gdt

#endif
