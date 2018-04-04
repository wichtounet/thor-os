//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DRIVER_HPET_H
#define DRIVER_HPET_H

#include <types.hpp>

namespace hpet {

bool install();
void late_install();

void init();

uint64_t counter();

} //end of namespace hpet

#endif
