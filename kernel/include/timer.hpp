//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TIMER_H
#define TIMER_H

#include <types.hpp>

namespace timer {

void install();

uint64_t ticks();
uint64_t seconds();

void tick();

/*!
 * \brief Return the frequency in Hz of the current timer system.
 */
uint64_t frequency();

/*!
 * \brief Sets the frequency in Hz of the current timer system.
 */
void frequency(uint64_t freq);

} //end of timer namespace

#endif
