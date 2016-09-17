//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "conc/sleep_queue.hpp"

#include "scheduler.hpp"
#include "logging.hpp"
#include "assert.hpp"

bool sleep_queue::empty() const {
    std::lock_guard<spinlock> l(lock);

    return queue.empty();
}

scheduler::pid_t sleep_queue::top_process() const {
    std::lock_guard<spinlock> l(lock);

    return queue.top();
}

scheduler::pid_t sleep_queue::wake_up() {
    std::lock_guard<spinlock> l(lock);

    while (!queue.empty()) {
        // Get the first process
        auto pid = queue.top();

        // Remove it
        queue.pop();

        if (pid != scheduler::INVALID_PID) {
            logging::logf(logging::log_level::TRACE, "sleep_queue: wake %d\n", pid);

            // Indicate to the scheduler that this process will be able to run
            // We use a hint here because it is possible that the thread was
            // already woken up from sleep
            scheduler::unblock_process_hint(pid);

            return pid;
        }
    }

    return scheduler::INVALID_PID;
}

void sleep_queue::wake_up_all() {
    std::lock_guard<spinlock> l(lock);

    while (!queue.empty()) {
        // Get the first process
        auto pid = queue.top();

        // Remove it
        queue.pop();

        if (pid != scheduler::INVALID_PID) {
            logging::logf(logging::log_level::TRACE, "sleep_queue: wake(all) %d\n", pid);

            // Indicate to the scheduler that this process will be able to run
            // We use a hint here because it is possible that the thread was
            // already woken up from sleep
            scheduler::unblock_process_hint(pid);
        }
    }
}

void sleep_queue::sleep() {
    lock.acquire();

    //Get the current process information
    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "sleep_queue: wait %d\n", pid);

    //Enqueue the process in the sleep queue
    queue.push(pid);

    thor_assert(!queue.full(), "The sleep_queue queue is full!");

    //This process will sleep
    scheduler::block_process_light(pid);

    lock.release();

    scheduler::reschedule();
}

bool sleep_queue::sleep(size_t ms) {
    if (!ms) {
        return false;
    }

    lock.acquire();

    //Get the current process information
    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "sleep_queue: %u wait with timeout %u\n", pid, ms);

    //Enqueue the process in the sleep queue
    queue.push(pid);

    thor_assert(!queue.full(), "The sleep_queue queue is full!");

    //This process will sleep
    scheduler::block_process_timeout_light(pid, ms);

    lock.release();

    scheduler::reschedule();

    // At this point we need the lock again to check the queue
    lock.acquire();

    bool obtained = true;

    // If the queue still contains our pid, it means a wake up
    // from timeout

    if (!queue.empty()) {
        // If the pid is on top, pop it
        if (queue.top() == pid) {
            obtained = false;
            queue.pop();
        }
        // If the pid is inside the queue, replace it with an invalid pid
        // If this happens too often, we'll need a better pid queue
        else if (queue.contains(pid)) {
            obtained = false;
            queue.replace(pid, scheduler::INVALID_PID);
        }
    }

    // Final release of the lock
    lock.release();

    return obtained;
}
