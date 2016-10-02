//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STL_STACK_H
#define STL_STACK_H

#include "stl/vector.hpp"
#include "stl/types.hpp"

namespace std {

/*!
 * \brief Container adapter to provide a stack (LIFO)
 */
template<typename T, typename C = std::vector<T>>
struct stack {
    /*!
     * \brief Indicates if the stack is empty
     */
    bool empty() const {
        return size() == 0;
    }

    /*!
     * \brief Returns the size of the stack
     */
    size_t size() const {
        return container.size();
    }

    /*!
     * \brief Adds the element at the top of the stack
     * \param value The element to add on the stack
     */
    void push(const T& value){
        container.push_back(value);
    }

    /*!
     * \brief Create a new element inplace onto the stack
     */
    template<typename... Args>
    void emplace(Args&&... args){
        container.emplace_back(std::forward<Args>(args)...);
    }

    /*!
     * \brief Removes the element at the top of the stack
     */
    void pop(){
        container.pop_back();
    }

    /*!
     * \brief Returns a reference to the element at the top of the stack
     */
    T& top(){
        return container.back();
    }

    /*!
     * \brief Returns a const reference to the element at the top of the stack
     */
    const T& top() const {
        return container.back();
    }

private:
    C container; ///< The underlying container
};

} //end of namespace std

#endif
