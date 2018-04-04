//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STL_QUEUE_H
#define STL_QUEUE_H

#include <deque.hpp>
#include <types.hpp>

namespace std {

/*!
 * \brief Container adapter to provide a queue (FIFO)
 */
template<typename T, typename C = std::deque<T>>
struct queue {
    using reference_type = typename C::reference_type;
    using const_reference_type = typename C::const_reference_type;

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
    void push(T&& value){
        container.push_back(std::move(value));
    }

    /*!
     * \brief Push a new element onto the queue
     */
    void push(const T& value){
        container.push_back(value);
    }

    /*!
     * \brief Create a new element inplace onto the queue
     */
    template<typename... Args>
    reference_type emplace(Args&&... args){
        return container.emplace_back(std::forward<Args>(args)...);
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
    reference_type top(){
        return container.front();
    }

    /*!
     * \brief Returns a const reference to the top element
     */
    const_reference_type top() const {
        return container.front();
    }

private:
    C container; ///< The underlying container
};

} //end of namespace std

#endif
