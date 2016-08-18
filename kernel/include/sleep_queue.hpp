//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SLEEP_QUEUE_H
#define SLEEP_QUEUE_H

#include <circular_buffer.hpp>
#include <lock_guard.hpp>

#include "spinlock.hpp"
#include "scheduler.hpp"
#include "logging.hpp"

/*!
 * \brief A simple sleep queue
 */
struct sleep_queue {
private:
    mutable spinlock lock;
    circular_buffer<scheduler::pid_t, 16> queue;

public:
    /*!
     * \brief Test if the sleep queue is empty
     */
    bool empty() const {
        std::lock_guard<spinlock> l(lock);

        return queue.empty();
    }

    /*!
     * \brief Returns the top process in the queue.
     *
     * If the queue is empty, the behaviour is undefined.
     */
    scheduler::pid_t top_process() const {
        std::lock_guard<spinlock> l(lock);

        return queue.top();
    }

    /*!
     * \brief Wake up the first process from the queue.
     *
     * If the queue is empty, the behaviour is undefined.
     */
    scheduler::pid_t wake_up(){
        std::lock_guard<spinlock> l(lock);

        while(!queue.empty()){
            // Get the first process
            auto pid = queue.top();

            // Remove it
            queue.pop();

            if(pid != scheduler::INVALID_PID){
                logging::logf(logging::log_level::TRACE, "sleep_queue: wake %d\n", pid);

                //Indicate to the scheduler that this process will be able to run
                scheduler::unblock_process(pid);

                return pid;
            }
        }

        return scheduler::INVALID_PID;
    }

    /*!
     * \brief Wait inside the queue until woken up.
     */
    void sleep(){
        lock.acquire();

        //Get the current process information
        auto pid = scheduler::get_pid();

        logging::logf(logging::log_level::TRACE, "sleep_queue: wait %d\n", pid);

        //Enqueue the process in the sleep queue
        queue.push(pid);

        //This process will sleep
        scheduler::block_process_light(pid);

        lock.release();

        scheduler::reschedule();
    }

    /*!
     * \brief Wait inside the queue until woken up or until the
     * timeout is passed.
     *
     * \return true if the thread was woken up, false if the timeout is passed
     */
    bool sleep(size_t ms){
        if(!ms){
            return false;
        }

        lock.acquire();

        //Get the current process information
        auto pid = scheduler::get_pid();

        logging::logf(logging::log_level::TRACE, "sleep_queue: %u wait with timeout %u\n", pid, ms);

        //Enqueue the process in the sleep queue
        queue.push(pid);

        //This process will sleep
        scheduler::block_process_timeout_light(pid, ms);

        lock.release();

        scheduler::reschedule();

        // At this point we need the lock again to check the queue
        lock.acquire();

        bool obtained = true;

        // If the queue still contains our pid, it means a wake up
        // from timeout
        if(queue.contains(pid)){
            obtained = false;
            queue.replace(pid, scheduler::INVALID_PID);
        }

        // Final release of the lock
        lock.release();

        return obtained;
    }
};

#endif
