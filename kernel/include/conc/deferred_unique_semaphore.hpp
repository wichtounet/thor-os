//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DEFERRED_UNIQUE_SEMAPHORE_H
#define DEFERRED_UNIQUE_SEMAPHORE_H

#include "conc/int_lock.hpp"

#include "scheduler.hpp"

/*!
 * \brief A deferred unique semaphore lock implementation.
 *
 * This must be used when the notifier is an IRQ handler.
 *
 * Once the lock is acquired, the critical section is only accessible by the
 * thread who acquired the mutex.
 */
struct deferred_unique_semaphore {
    /*!
     * \brief Claim the mutex
     */
    void claim() {
        this->pid = scheduler::get_pid();
    }

    void init(size_t value){
        this->value = value;
    }

    /*!
     * \brief Wait for the lock
     */
    void wait() {
        int_lock lock;

        lock.lock();

        if (value > 0) {
            --value;

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
     * \brief Release the lock, from an IRQ handler.
     */
    void notify() {
        if (!waiting) {
            ++value;
        } else {
            scheduler::unblock_process_hint(pid);
        }
    }

    /*!
     * \brief Release the lock several times, from an IRQ
     */
    void notify(size_t n) {
        if (!waiting) {
            value += n;
        } else {
            scheduler::unblock_process_hint(pid);
            --n;

            if (n) {
                value += n;
            }
        }
    }

private:
    size_t pid            = 0;     ///< The claimed pid
    volatile size_t value = 0;     ///< The value of the mutex
    volatile bool waiting = false; ///< Indicates if the process is waiting
};

#endif
