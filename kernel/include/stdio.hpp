//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STDIO_H
#define STDIO_H

#include <types.hpp>

#include "terminal.hpp"

namespace stdio {

void init_terminals();
void register_devices();
void finalize();

void switch_terminal(size_t id);

size_t terminals_count();
virtual_terminal& get_active_terminal();
virtual_terminal& get_terminal(size_t id);

} //end of namespace stdio

#endif
