//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VECTOR_H
#define VECTOR_H

#include "stl/types.hpp"
#include "stl/algorithms.hpp"

namespace std {

template<typename T>
class vector {
public:
    typedef T                       value_type;
    typedef value_type*             pointer_type;
    typedef size_t                  size_type;
    typedef value_type*             iterator;
    typedef const value_type*       const_iterator;

private:
    T* data;
    uint64_t _size;
    uint64_t _capacity;

public:
    vector() : data(nullptr), _size(0), _capacity(0) {}
    explicit vector(uint64_t c) : data(new T[c]), _size(0), _capacity(c) {}

    // Disable copy for now
    vector(const vector& rhs) = delete;
    vector& operator=(const vector& rhs) = delete;

    //Move constructors

    vector(vector&& rhs) : data(rhs.data), _size(rhs._size), _capacity(rhs._capacity) {
        rhs.data = nullptr;
        rhs._size = 0;
        rhs._capacity = 0;
    };

    vector& operator=(vector&& rhs){
        data = rhs.data;
        _size = rhs._size;
        _capacity = rhs._capacity;
        rhs.data = nullptr;
        rhs._size = 0;
        rhs._capacity = 0;

        return *this;
    }

    ~vector(){
        if(data){
            delete[] data;
        }
    }

    //Getters

    constexpr size_type size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    constexpr size_type capacity() const {
        return _capacity;
    }

    constexpr const value_type& operator[](size_type pos) const {
        return data[pos];
    }

    value_type& operator[](size_type pos){
        return data[pos];
    }

    //Modifiers

    void push_back(const value_type& element){
        if(_capacity == 0){
            _capacity = 1;
            data = new T[_capacity];
        } else if(_capacity == _size){
            _capacity= _capacity * 2;

            auto new_data = new T[_capacity];
            std::move_n(new_data, data, _size);

            delete[] data;
            data = new_data;
        }

        data[_size++] = element;
    }

    void pop_back(){
        --_size;
    }

    void clear(){
        _size = 0;
    }

    //Iterators

    iterator begin(){
        return iterator(&data[0]);
    }

    constexpr const_iterator begin() const {
        return const_iterator(&data[0]);
    }

    iterator end(){
        return iterator(&data[_size]);
    }

    constexpr const_iterator end() const {
        return const_iterator(&data[_size]);
    }
};

} //end of namespace std

#endif
