//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <lock_guard.hpp>

#include "conc/condition_variable.hpp"

#include "scheduler.hpp"
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

    if (!queue.empty()) {
        // Here we must use a hint since the process may have
        // used a timeout and may have been woken up by the
        // scheduler but not yet removed its pid from the queue
        return queue.dequeue_hint();
    }

    return scheduler::INVALID_PID;
}

void condition_variable::notify_all() {
    std::lock_guard<spinlock> l(lock);

    while (!queue.empty()) {
        // Here we must use a hint since the process may have
        // used a timeout and may have been woken up by the
        // scheduler but not yet removed its pid from the queue
        queue.dequeue_hint();
    }
}

void condition_variable::wait() {
    lock.lock();

    //Enqueue the process in the sleep queue
    queue.enqueue();

    lock.unlock();

    scheduler::reschedule();
}

bool condition_variable::wait_for(size_t ms) {
    if (!ms) {
        return false;
    }

    lock.lock();

    //Enqueue the process in the sleep queue
    queue.enqueue_timeout(ms);

    lock.unlock();

    scheduler::reschedule();

    // At this point we need the lock again to check the queue
    lock.lock();

    // If the queue still contains our pid, it means a wake up
    // from timeout

    if(queue.waiting()){
        queue.remove();

        // Final release of the lock
        lock.unlock();

        return false;
    }

    // Final release of the lock
    lock.unlock();

    return true;
}
