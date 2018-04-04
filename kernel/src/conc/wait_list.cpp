//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "conc/wait_list.hpp"

#include "scheduler.hpp"

bool wait_list::empty() const {
    return !head;
}

size_t wait_list::top() const {
    return head->pid;
}

bool wait_list::waiting() const {
    auto pid      = scheduler::get_pid();
    auto& process = scheduler::get_process(pid);

    auto node = head;

    while(node){
        if(node == &process.wait){
            return true;
        }

        node = node->next;
    }

    return false;
}

void wait_list::remove(){
    auto pid      = scheduler::get_pid();
    auto& process = scheduler::get_process(pid);

    if (head == &process.wait) {
        if(head == tail){
            tail = head = nullptr;
        } else {
            head = head->next;
        }

        return;
    }

    auto node = head;

    while(node->next){
        if(node->next == &process.wait){
            if(tail == node->next){
                tail = node->next->next;
            }

            node->next = node->next->next;

            return;
        }

        node = node->next;
    }
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

    scheduler::block_process_light(pid);
}

void wait_list::enqueue_timeout(size_t ms) {
    auto pid      = scheduler::get_pid();
    auto& process = scheduler::get_process(pid);

    process.wait.next = nullptr;

    if (!tail) {
        tail = head = &process.wait;
    } else {
        tail = tail->next = &process.wait;
    }

    scheduler::block_process_timeout_light(pid, ms);
}

size_t wait_list::dequeue() {
    auto pid = head->pid;

    if (head == tail) {
        head = tail = nullptr;
    } else {
        head = head->next;
    }

    scheduler::unblock_process(pid);

    return pid;
}

size_t wait_list::dequeue_hint() {
    auto pid = head->pid;

    if (head == tail) {
        head = tail = nullptr;
    } else {
        head = head->next;
    }

    scheduler::unblock_process_hint(pid);

    return pid;
}
