//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <types.hpp>
#include <utility.hpp>

/*!
 * \brief A circular buffer of maximum size S
 *
 * In a circular buffer, push are done on one side of the queue and
 * pop on the other side (FIFO)
 */
template<typename T, size_t S>
struct circular_buffer {
private:
    static constexpr const size_t Size = S + 1;

    T buffer[Size];

    volatile size_t start;
    volatile size_t end;

public:
    /*!
     * \brief Construct a new circular_buffer
     */
    circular_buffer() : start(0), end(0) {
        //Nothing to init
    }

    /*!
     * \brief Returns true if the buffer is full
     */
    bool full() const {
        return (end + 1) % Size == start;
    }

    /*!
     * \brief Returns true if the buffer is empty
     */
    bool empty() const {
        return end == start;
    }

    /*!
     * \brief Push the given value to the buffer
     * \param value The value
     */
    bool push(T value){
        if(full()){
            return false;
        } else {
            buffer[end] = value;
            end = (end + 1) % Size;

            return true;
        }
    }

    /*!
     * \brief Construct a new element in place at it would have been pushed
     * \param values The values to forward to the constructor
     */
    template<typename... Ts>
    bool emplace_push(Ts&&... values){
        if(full()){
            return false;
        } else {
            new (&buffer[end]) T{std::forward<Ts>(values)...};
            end = (end + 1) % Size;

            return true;
        }
    }

    /*!
     * \brief Returns the value at the top of the buffer
     */
    T top() const {
        return buffer[start];
    }

    /*!
     * \brief Removes and returns the value at the top of the buffer
     */
    T pop(){
        auto value = buffer[start];
        start = (start + 1) % Size;
        return value;
    }

    /*!
     * \brief Removes the last element that was pushed
     *
     * This should only be used if the buffer is used by a single
     * thread.
     */
    void pop_last(){
        if(end == 0){
            end = Size - 1;
        } else {
            --end;
        }
    }

    /*!
     * \brief Test if the buffer contains the given value.
     *
     * This should only be used if the buffer is used by a single
     * thread.
     */
    bool contains(const T& value){
        for(size_t i = start; i != end; i = (i + 1) % Size){
            if(buffer[i] == value){
                return true;
            }
        }

        return false;
    }

    /*!
     * \brief Replace the searched value by the given value
     *
     * This should only be used if the buffer is used by a single
     * thread.
     */
    void replace(const T& value, const T& new_value){
        for(size_t i = start; i != end; i = (i + 1) % Size){
            if(buffer[i] == value){
                buffer[i] = new_value;
                return;
            }
        }
    }
};

#endif
