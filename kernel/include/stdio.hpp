//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STDIO_H
#define STDIO_H

#include <types.hpp>

#include "terminal.hpp"

namespace stdio {

/*!
 * \brief Initialize the terminals
 */
void init_terminals();

/*!
 * \brief Register the devices into the devfs
 */
void register_devices();

/*!
 * \brief Finalize the terminals
 */
void finalize();

/*!
 * \brief Switch the active terminal to the terminal with the given id
 * \param id The terminal to activate
 */
void switch_terminal(size_t id);

/*!
 * \brief Returns the number of terminals
 */
size_t terminals_count();

/*!
 * \brief Returns the active terminal
 */
virtual_terminal& get_active_terminal();

/*!
 * \brief Returns the terminal with the given id
 */
virtual_terminal& get_terminal(size_t id);

} //end of namespace stdio

#endif
