//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STL_QUEUE_H
#define STL_QUEUE_H

#include <list.hpp>
#include <types.hpp>

namespace std {

/*!
 * \brief Container adapter to provide a queue (FIFO)
 */
template<typename T, typename C = std::list<T>>
struct queue {
    /*!
     * \brief Indicates if the queue is empty
     */
    bool empty() const {
        return size() == 0;
    }

    /*!
     * \brief Returns the size of the queue
     */
    size_t size() const {
        return container.size();
    }

    /*!
     * \brief Push a new element onto the queue
     */
    void push(const T& value){
        container.push_back(value);
    }

    /*!
     * \brief Pop the top element from the queue
     */
    void pop(){
        container.pop_front();
    }

    /*!
     * \brief Returns a reference to the top element
     */
    T& top(){
        return container.front();
    }

    /*!
     * \brief Returns a const reference to the top element
     */
    const T& top() const {
        return container.front();
    }

private:
    C container; ///< The underlying container
};

} //end of namespace std

#endif
