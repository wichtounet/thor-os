//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TIMER_H
#define TIMER_H

#include "stl/types.hpp"

void install_timer();
void sleep_ms(uint64_t delay);

uint64_t timer_ticks();
uint64_t timer_seconds();

#endif
