//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef ARRAY_H
#define ARRAY_H

#include <types.hpp>

namespace std {

/*!
 * \brief A fixed-size array
 */
template<typename T, size_t N>
struct array {
    using value_type = T; ///< The value type
    using iterator = value_type*; ///< The iterator type
    using const_iterator = const value_type*; ///< The const iterator type
    using size_type = size_t; ///< The size type

    /*!
     * \brief Returns a reference to the element at the given position
     */
    T& operator[](size_type pos){
        return __data[pos];
    }

    /*!
     * \brief Returns a const reference to the element at the given position
     */
    constexpr const T& operator[](size_type pos) const {
        return __data[pos];
    }

    /*!
     * \brief Returns the size of the array
     */
    constexpr size_type size() const {
        return N;
    }

    /*!
     * \brief Returns an iterator poiting to the first element of the array
     */
    iterator begin(){
        return iterator(&__data[0]);
    }

    /*!
     * \brief Returns a const iterator poiting to the first element of the array
     */
    const_iterator begin() const {
        return const_iterator(&__data[0]);
    }

    /*!
     * \brief Returns an iterator poiting to the past-the-end element of the array
     */
    iterator end(){
        return iterator(&__data[N]);
    }

    /*!
     * \brief Returns a const iterator poiting to the past-the-end element of the array
     */
    const_iterator end() const {
        return const_iterator(&__data[N]);
    }

    /*!
     * \brief Returns the underlying data storage
     */
    value_type* data(){
        return __data;
    }

private:
    T __data[N]; ///< The stored data
};

template<typename T>
class unique_heap_array {
public:
    typedef T                       value_type;
    typedef value_type*             pointer_type;
    typedef value_type*             iterator;
    typedef const value_type*       const_iterator;
    typedef size_t                size_type;

private:
    T* array;
    size_t _size;

public:
    unique_heap_array() : array(nullptr), _size(0) {}

    explicit unique_heap_array(T* a, size_type s) : array(a), _size(s) {}
    explicit unique_heap_array(size_type s) : _size(s) {
        array = new T[s];
    }

    unique_heap_array(unique_heap_array&& u) : array(u.unlock()), _size(u._size) {
        u._size = 0;
    }

    unique_heap_array& operator=(unique_heap_array&& u){
        _size = u._size;
        reset(u.unlock());
        u._size = 0;
        return *this;
    }

    ~unique_heap_array(){
        reset();
        _size = 0;
    }

    // Disable copy
    unique_heap_array(const unique_heap_array& rhs) = delete;
    unique_heap_array& operator=(const unique_heap_array& rhs) = delete;

    /*!
     * \brief Return the size of the array
     */
    size_type size() const {
        return _size;
    }

    /*!
     * \brief Returns a const reference to the element at the given position
     */
    const T& operator[](size_type pos) const {
        return array[pos];
    }

    /*!
     * \brief Returns a reference to the element at the given position
     */
    T& operator[](size_type pos){
        return array[pos];
    }

    pointer_type get(){
        return array;
    }

    pointer_type unlock(){
        pointer_type p = array;
        array = nullptr;
        return p;
    }

    void reset(pointer_type p = pointer_type()){
        if(array != p){
            delete[] array;
            array= p;
        }
    }

    /*!
     * \brief Returns an iterator poiting to the first element of the array
     */
    iterator begin(){
        return iterator(&array[0]);
    }

    /*!
     * \brief Returns a const iterator poiting to the first element of the array
     */
    const_iterator begin() const {
        return const_iterator(&array[0]);
    }

    /*!
     * \brief Returns an iterator poiting to the past-the-end element of the array
     */
    iterator end(){
        return iterator(&array[_size]);
    }

    /*!
     * \brief Returns a const iterator poiting to the past-the-end element of the array
     */
    const_iterator end() const {
        return const_iterator(&array[_size]);
    }
};

} //end of namespace std

#endif
