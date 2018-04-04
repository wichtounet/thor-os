//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DEFERRED_UNIQUE_MUTEX_H
#define DEFERRED_UNIQUE_MUTEX_H

#include "conc/int_lock.hpp"

#include "scheduler.hpp"

/*!
 * \brief A deferred unique mutex lock implementation.
 *
 * This must be used when the notifier is an IRQ handler.
 *
 * Once the lock is acquired, the critical section is only accessible by the
 * thread who acquired the mutex.
 */
struct deferred_unique_mutex {
    /*!
     * \brief Claim the mutex
     */
    void claim() {
        this->pid = scheduler::get_pid();
    }

    /*!
     * \brief Wait for the lock
     */
    void wait() {
        int_lock lock;

        lock.lock();

        if (value > 0) {
            value = 0;

            lock.unlock();
        } else {
            waiting = true;

            scheduler::block_process_light(pid);
            lock.unlock();
            scheduler::reschedule();

            waiting = false;
        }
    }

    /*!
     * \brief Release the lock, from an IRQ
     */
    void notify() {
        if (!waiting) {
            value = 1;
        } else {
            scheduler::unblock_process_hint(pid);
        }
    }

private:
    size_t pid            = 0;     ///< The claimed pid
    volatile size_t value = 0;     ///< The value of the mutex
    volatile bool waiting = false; ///< Indicates if the process is waiting
};

#endif
