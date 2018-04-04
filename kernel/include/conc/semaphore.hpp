//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <types.hpp>
#include <lock_guard.hpp>

#include "conc/spinlock.hpp"
#include "conc/wait_list.hpp"

#include "scheduler.hpp"

/*!
 * \brief A semaphore implementation.
 *
 * The critical section can be open to several processes.
 */
struct semaphore {
    /*!
     * \brief Initialize the semaphore
     * \param v The intial value of the semaphore
     */
    void init(size_t v) {
        value = v;
    }

    /*!
     * \brief Acquire the lock.
     *
     * This will effectively decrease the current counter by 1 once the critical
     * section is entered.
     */
    void lock() {
        value_lock.lock();

        if (value > 0) {
            --value;
            value_lock.unlock();
        } else {
            queue.enqueue();

            value_lock.unlock();
            scheduler::reschedule();
        }
    }

    /*!
     * \brief Try to acquire the lock.
     *
     * This function returns immediately.
     *
     * \return true if the lock was acquired, false otherwise.
     */
    bool try_lock() {
        std::lock_guard<spinlock> l(value_lock);

        if (value > 0) {
            --value;

            return true;
        } else {
            return false;
        }
    }

    /*!
     * \brief Release the lock.
     *
     * This will effectively increase the current counter by 1 once the critical
     * section is left.
     */
    void unlock() {
        std::lock_guard<spinlock> l(value_lock);

        if (queue.empty()) {
            ++value;
        } else {
            // Wake up the process
            queue.dequeue();

            //No need to increment value, the process won't
            //decrement it
        }
    }

    /*!
     * \brief Release the lock several times.
     *
     * This will effectively increase the current counter by n once the critical
     * section is left.
     */
    void release(size_t n) {
        std::lock_guard<spinlock> l(value_lock);

        if (queue.empty()) {
            value += n;
        } else {
            while (n && !queue.empty()) {
                queue.dequeue();
                --n;
            }

            if (n) {
                value += n;
            }
        }
    }

private:
    spinlock value_lock;    ///< The spin lock protecting the counter
    volatile size_t value;  ///< The value of the counter
    wait_list queue;        ///< The sleep queue
};

#endif
