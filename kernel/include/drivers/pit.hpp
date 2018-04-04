//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DRIVER_PIT_H
#define DRIVER_PIT_H

#include <types.hpp>

namespace pit {

bool install();
void remove();

uint64_t counter();

} //end of namespace pit

#endif
