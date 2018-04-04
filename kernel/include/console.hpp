//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdarg.h>

#include <types.hpp>
#include <enable_if.hpp>
#include <string.hpp>

namespace stdio {

/*!
 * \brief Init the console
 */
void init_console();

/*!
 * \brief A console
 */
struct console {
    /*!
     * \brief Init the console
     */
    void init();

    /*!
     * \brief Returns the number of columns of the console
     */
    size_t get_columns() const;

    /*!
     * \brief Returns the number of rows of the console
     */
    size_t get_rows() const;

    /*!
     * \brief Print the given char to the console
     * \param c The character to print
     */
    void print(char c);

    /*!
     * \brief Clear the console
     */
    void wipeout();

    /*!
     * \brief Set the active status of the console
     * \param active The active status of the console
     */
    void set_active(bool active);

    /*!
     * \brief Save the state of the console
     */
    void save();

    /*!
     * \brief Restore the state of the console
     */
    void restore();

private:
    /*!
     * \brief Move to the next line
     */
    void next_line();

    size_t current_line   = 0; ///< The current line of the console
    size_t current_column = 0; ///< The current column of the console

    void* buffer = nullptr; ///< The buffer to save the state
    bool active  = false;   ///< The active status of the console
};

} // end of namespace stdio

#endif
