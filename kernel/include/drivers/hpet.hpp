//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
