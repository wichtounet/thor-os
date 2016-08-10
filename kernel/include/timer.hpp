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

/*!
 * \brief Install the system timer
 */
void install();

/*!
 * \brief Returns a up-counter in timer ticks
 */
uint64_t ticks();

/*!
 * \brief Returns a up-counter in seconds
 */
uint64_t seconds();

/*!
 * \brief Returns a up-counter in milliseconds
 */
uint64_t milliseconds();

/*!
 * \brief Let the timer know of a new tick
 */
void tick();

/*!
 * \brief Return the frequency in Hz of the current timer system.
 */
uint64_t timer_frequency();

/*!
 * \brief Sets the frequency in Hz of the current timer system.
 */
void timer_frequency(uint64_t freq);

} //end of timer namespace

#endif
