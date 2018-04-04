//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <types.hpp>

namespace mouse {

/*!
 * \brief Install the mouse driver
 */
void install();

/*!
 * \brief Returns the x position of the mouse
 */
uint64_t x();

/*!
 * \brief Returns the y position of the mouse
 */
uint64_t y();

} //end of namespace mouse

#endif
