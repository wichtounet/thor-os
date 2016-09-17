//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef ARRAY_H
#define ARRAY_H

#include <types.hpp>

namespace std {

template<typename T, size_t N>
class array {
private:
    T __data[N];

public:
    typedef T                       value_type;
    typedef value_type*             iterator;
    typedef const value_type*       const_iterator;
    typedef size_t                size_type;

    T& operator[](size_type pos){
        return __data[pos];
    }

    constexpr const T& operator[](size_type pos) const {
        return __data[pos];
    }

    constexpr size_type size() const {
        return N;
    }

    iterator begin(){
        return iterator(&__data[0]);
    }

    const_iterator begin() const {
        return const_iterator(&__data[0]);
    }

    iterator end(){
        return iterator(&__data[N]);
    }

    const_iterator end() const {
        return const_iterator(&__data[N]);
    }

    value_type* data(){
        return __data;
    }
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

    size_type size() const {
        return _size;
    }

    const T& operator[](size_type pos) const {
        return array[pos];
    }

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

    iterator begin(){
        return iterator(&array[0]);
    }

    const_iterator begin() const {
        return const_iterator(&array[0]);
    }

    iterator end(){
        return iterator(&array[_size]);
    }

    const_iterator end() const {
        return const_iterator(&array[_size]);
    }
};

} //end of namespace std

#endif
