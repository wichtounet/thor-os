//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <circular_buffer.hpp>
#include <lock_guard.hpp>

#include "spinlock.hpp"
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
    void init(size_t v){
        value = v;
    }

    /*!
     * \brief Acquire the lock.
     *
     * This will effectively decrease the current counter by 1 once the critical
     * section is entered.
     */
    void acquire(){
        lock.acquire();

        if(value > 0){
            --value;
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
     * \brief Release the lock.
     *
     * This will effectively increase the current counter by 1 once the critical
     * section is left.
     */
    void release(){
        std::lock_guard<spinlock> l(lock);

        if(queue.empty()){
            ++value;
        } else {
            // Wake up the process
            auto pid = queue.pop();
            scheduler::unblock_process(pid);

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
    void release(size_t n){
        std::lock_guard<spinlock> l(lock);

        if(queue.empty()){
            value += n;
        } else {
            while(n && !queue.empty()){
                auto pid = queue.pop();
                scheduler::unblock_process(pid);
                --n;
            }

            if(n){
                value += n;
            }
        }
    }

private:
    mutable spinlock lock;                       ///< The spin lock protecting the counter
    volatile size_t value;                       ///< The value of the counter
    circular_buffer<scheduler::pid_t, 16> queue; ///< The sleep queue
};

#endif
