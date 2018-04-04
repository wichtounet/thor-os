//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef VESA_CONSOLE_H
#define VESA_CONSOLE_H

#include <types.hpp>

/*!
 * \brief A VESA console
 */
struct vesa_console {
    /*!
     * \brief Initialize the console
     */
    void init();

    /*!
     * \brief Returns the number of lines of the console
     */
    size_t lines() const;

    /*!
     * \brief Returns the number of columns of the console
     */
    size_t columns() const;

    /*!
     * \brief Clear the vesa console
     */
    void clear();

    /*!
     * \brief Clear the given vesa console state
     * \param buffer The VESA console state
     */
    void clear(void* buffer);

    /*!
     * \brief Scroll up one line
     */
    void scroll_up();

    /*!
     * \brief Scroll up one line on the given vesa console state
     * \param buffer The VESA console state
     */
    void scroll_up(void* buffer);

    /*!
     * \brief Print a char at the given line and column
     * \param line The line at which to print the char
     * \param column The column at which to print the char
     * \param c The char to print
     */
    void print_char(size_t line, size_t column, char c);

    /*!
     * \brief Print a char at the given line and column on the given vesa console state
     * \param buffer The VESA console state
     * \param line The line at which to print the char
     * \param column The column at which to print the char
     * \param c The char to print
     */
    void print_char(void* buffer, size_t line, size_t column, char c);

    /*!
     * \brief Save the state of the console
     * \param buffer The buffer to save to
     * \return the buffer the state was saved to
     */
    void* save(void* buffer);

    /*!
     * \brief Restore the state of the console
     * \param buffer The buffer to restore from
     */
    void restore(void* buffer);
};

#endif
