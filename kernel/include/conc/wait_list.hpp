//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef WAIT_LIST_H
#define WAIT_LIST_H

#include <types.hpp>

struct wait_node {
    size_t pid;
    wait_node* next;
};

/*!
 * \brief A list of processes waiting.
 *
 * It is implemented as an intrusive singly linked list.
 */
struct wait_list {
    /*!
     * \brief Test if the list is empty.
     */
    bool empty() const;

    /*!
     * \brief Returns the top process of queue
     * \return The pid of the top process
     */
    size_t top() const;

    /*!
     * \brief Returns true if the process is waiting in this queue, false otherwise
     */
    bool waiting() const;

    /*!
     * \brief Removes the current process from the list.
     *
     * The process must be in the list!
     */
    void remove();

    /*!
     * \brief Enque the current process in the wait list
     */
    void enqueue();

    /*!
     * \brief Enque the current process in the wait list
     */
    void enqueue_timeout(size_t ms);

    /*!
     * \brief Dequeue the first process from the wait list
     * \return The pid of the dequeued process
     */
    size_t dequeue();

    /*!
     * \brief Dequeue the first process from the wait list
     * \return The pid of the dequeued process
     */
    size_t dequeue_hint();

private:
    wait_node* head = nullptr; ///< The head of the list
    wait_node* tail = nullptr; ///< The tail of the list
};

#endif
