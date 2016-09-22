//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "conc/condition_variable.hpp"

#include "scheduler.hpp"
#include "logging.hpp"
#include "assert.hpp"

bool condition_variable::empty() const {
    std::lock_guard<spinlock> l(lock);

    return queue.empty();
}

scheduler::pid_t condition_variable::top_process() const {
    std::lock_guard<spinlock> l(lock);

    return queue.top();
}

scheduler::pid_t condition_variable::notify_one() {
    std::lock_guard<spinlock> l(lock);

    while (!queue.empty()) {
        // Get the first process
        auto pid = queue.top();

        // Remove it
        queue.pop();

        if (pid != scheduler::INVALID_PID) {
            logging::logf(logging::log_level::TRACE, "condition_variable: wake %d\n", pid);

            // Here we must use a hint since the process may have
            // used a timeout and may have been woken up by the
            // scheduler but not yet removed its pid from the queue
            scheduler::unblock_process_hint(pid);

            return pid;
        }
    }

    return scheduler::INVALID_PID;
}

void condition_variable::notify_all() {
    std::lock_guard<spinlock> l(lock);

    while (!queue.empty()) {
        // Get the first process
        auto pid = queue.top();

        // Remove it
        queue.pop();

        if (pid != scheduler::INVALID_PID) {
            logging::logf(logging::log_level::TRACE, "condition_variable: wake(all) %d\n", pid);

            // Here we must use a hint since the process may have
            // used a timeout and may have been woken up by the
            // scheduler but not yet removed its pid from the queue
            scheduler::unblock_process_hint(pid);
        }
    }
}

void condition_variable::wait() {
    lock.lock();

    //Get the current process information
    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "condition_variable: wait %d\n", pid);

    //Enqueue the process in the sleep queue
    queue.push(pid);

    thor_assert(!queue.full(), "The condition_variable queue is full!");

    //This process will sleep
    scheduler::block_process_light(pid);

    lock.unlock();

    scheduler::reschedule();
}

bool condition_variable::wait_for(size_t ms) {
    if (!ms) {
        return false;
    }

    lock.lock();

    //Get the current process information
    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "condition_variable: %u wait with timeout %u\n", pid, ms);

    //Enqueue the process in the sleep queue
    queue.push(pid);

    thor_assert(!queue.full(), "The condition_variable queue is full!");

    //This process will sleep
    scheduler::block_process_timeout_light(pid, ms);

    lock.unlock();

    scheduler::reschedule();

    // At this point we need the lock again to check the queue
    lock.lock();

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
    lock.unlock();

    return obtained;
}
