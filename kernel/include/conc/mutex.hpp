//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef MUTEX_H
#define MUTEX_H

#include <circular_buffer.hpp>
#include <lock_guard.hpp>

#include "conc/spinlock.hpp"

#include "scheduler.hpp"
#include "logging.hpp"

/*!
 * \brief A mutex implementation.
 *
 * Once the lock is acquired, the critical section is only accessible by the
 * thread who acquired the mutex.
 */
struct mutex {
    /*!
     * \brief Initialize the mutex (either to 1 or 0)
     * \param v The intial value of the mutex
     */
    void init(size_t v = 1){
        if(v > 1){
            value = 1;
        } else {
            value = v;
        }
    }

    /*!
     * \brief Acquire the lock
     */
    void acquire(){
        lock.acquire();

        if(value > 0){
            value = 0;

            lock.release();
        } else {
            auto pid = scheduler::get_pid();
            queue.push(pid);

            scheduler::block_process_light(pid);
            lock.release();
            scheduler::reschedule();
        }
    }

    /*!
     * \brief Acquire the lock
     */
    void release(){
        std::lock_guard<spinlock> l(lock);

        if(queue.empty()){
            value = 1;
        } else {
            auto pid = queue.pop();
            scheduler::unblock_process(pid);

            //No need to increment value, the process won't
            //decrement it
        }
    }

private:
    mutable spinlock lock;                       ///< The spin protecting the value
    volatile size_t value;                       ///< The value of the mutex
    circular_buffer<scheduler::pid_t, 16> queue; ///< The sleep queue
};

#endif
