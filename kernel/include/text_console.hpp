//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TEXT_CONSOLE_H
#define TEXT_CONSOLE_H

#include <types.hpp>

/*!
 * \brief A textual console
 */
struct text_console {
    /*!
     * \brief Initialize the console
     */
    void init();

    /*!
     * \brief Returns the number of lines of the console
     */
    size_t lines();

    /*!
     * \brief Returns the number of columns of the console
     */
    size_t columns();

    /*!
     * \brief Clear the text console
     */
    void clear();

    /*!
     * \brief Scroll up one line
     */
    void scroll_up();

    /*!
     * \brief Print a char at the given line and column
     * \param line The line at which to print the char
     * \param column The column at which to print the char
     * \param c The char to print
     */
    void print_char(size_t line, size_t column, char c);
};

#endif
