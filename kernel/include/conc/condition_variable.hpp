//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H

#include <circular_buffer.hpp>
#include <lock_guard.hpp>

#include "conc/spinlock.hpp"

#include "process.hpp"

/*!
 * \brief A simple sleep queue
 */
struct condition_variable {
    /*!
     * \brief Test if the sleep queue is empty
     */
    bool empty() const ;

    /*!
     * \brief Returns the top process in the queue.
     *
     * If the queue is empty, the behaviour is undefined.
     */
    scheduler::pid_t top_process() const ;

    /*!
     * \brief Wake up the first process from the queue.
     */
    scheduler::pid_t wake_up();

    /*!
     * \brief Wake up all the processes from the queue.
     */
    void wake_up_all();

    /*!
     * \brief Wait inside the queue until woken up.
     */
    void sleep();

    /*!
     * \brief Wait inside the queue until woken up or until the
     * timeout is passed.
     *
     * \return true if the thread was woken up, false if the timeout is passed
     */
    bool sleep(size_t ms);

private:
    mutable spinlock lock;                       ///< The spin lock used for protecting the queue
    circular_buffer<scheduler::pid_t, 16> queue; ///< The queue of waiting threads
};

#endif
