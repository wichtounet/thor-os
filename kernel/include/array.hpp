//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ARRAY_H
#define ARRAY_H

#include "thor.hpp"

template<typename T, uint64_t N>
class array {
private:
    T data[N];

public:
    typedef T                       value_type;
    typedef value_type*             iterator;
    typedef const value_type*       const_iterator;
    typedef uint64_t                size_type;

    T& operator[](size_type pos){
        return data[pos];
    }

    const T& operator[](size_type pos) const {
        return data[pos];
    }

    size_type size(){
        return N;
    }

    iterator begin(){
        return iterator(&data[0]);
    }

    const_iterator begin() const {
        return const_iterator(&data[0]);
    }

    iterator end(){
        return iterator(&data[N]);
    }

    const_iterator end() const {
        return const_iterator(&data[N]);
    }
};

template<typename T>
class unique_heap_array {
public:
    typedef T                       value_type;
    typedef value_type*             pointer_type;
    typedef value_type*             iterator;
    typedef const value_type*       const_iterator;
    typedef uint64_t                size_type;

private:
    T* array;
    uint64_t _size;

public:
    unique_heap_array() : array(nullptr), _size(0) {}

    explicit unique_heap_array(T* a, size_type s) : array(a), _size(s) {}
    explicit unique_heap_array(size_type s) : _size(s) {
        array = new T[s];
    }

    unique_heap_array(unique_heap_array&& u) : array(u.release()), _size(u._size) {
        u._size = 0;
    }

    unique_heap_array& operator=(unique_heap_array&& u){
        _size = u._size;
        reset(u.release());
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

    pointer_type release(){
        pointer_type p = array;
        array = nullptr;
        return p;
    }

    void reset(pointer_type p = pointer_type()){
        if(array!= p){
            delete[] array;
            array= nullptr;
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

#endif
