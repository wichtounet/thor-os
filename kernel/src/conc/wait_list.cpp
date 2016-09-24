//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "conc/wait_list.hpp"

#include "scheduler.hpp"

bool wait_list::empty() const {
    return !head;
}

void wait_list::enqueue() {
    auto pid      = scheduler::get_pid();
    auto& process = scheduler::get_process(pid);

    process.wait.next = nullptr;

    if (!tail) {
        tail = head = &process.wait;
    } else {
        tail = tail->next = &process.wait;
    }
}

size_t wait_list::dequeue() {
    auto pid = head->pid;

    if (head == tail) {
        head = tail = nullptr;
    } else {
        head = head->next;
    }

    return pid;
}
