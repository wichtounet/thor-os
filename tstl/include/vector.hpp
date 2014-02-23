//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VECTOR_H
#define VECTOR_H

#include <types.hpp>
#include <algorithms.hpp>
#include <new.hpp>

//TODO The vector does not call any destructor

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

    vector(const vector& rhs) : data(nullptr), _size(rhs._size), _capacity(rhs._capacity) {
        if(!rhs.empty()){
            data = new T[_capacity];

            for(size_t i = 0; i < _size; ++i){
                data[i] = rhs.data[i];
            }
        }
    }

    vector& operator=(const vector& rhs){
        if(data && _capacity < rhs._capacity){
            delete[] data;
            data = nullptr;
        }

        _size = rhs._size;
        _capacity = rhs._capacity;

        if(!rhs.empty()){
            data = new T[_capacity];

            for(size_t i = 0; i < _size; ++i){
                data[i] = rhs.data[i];
            }
        }

        return *this;
    }

    //Move constructors

    vector(vector&& rhs) : data(rhs.data), _size(rhs._size), _capacity(rhs._capacity) {
        rhs.data = nullptr;
        rhs._size = 0;
        rhs._capacity = 0;
    };

    vector& operator=(vector&& rhs){
        if(data){
            delete[] data;
        }

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

    value_type& front(){
        return data[0];
    }

    const value_type& front() const  {
        return data[0];
    }

    value_type& back(){
        return data[size() - 1];
    }

    const value_type& back() const  {
        return data[size() - 1];
    }

    //Modifiers

    void reserve(size_t new_capacity){
        if(new_capacity > capacity()){
            ensure_capacity(new_capacity);
        }
    }

    void resize(size_t new_size){
        if(new_size > size()){
            ensure_capacity(new_size);

            //The elements will automatically be created to their defaults when
            //the array gets resized in ensure_capacity
            _size = new_size;
        } else if(new_size < _size){
            //By diminishing the size, the last elements become unreachable
            _size = new_size;
        }
    }

    void push_back(value_type&& element){
        ensure_capacity(_size + 1);

        data[_size++] = std::forward<value_type>(element);
    }

    void push_back(const value_type& element){
        ensure_capacity(_size + 1);

        data[_size++] = element;
    }

    template<typename... Args>
    void emplace_back(Args... args){
        ensure_capacity(_size + 1);

        new (&data[_size++]) T(std::forward<Args>(args)...);
    }

    void pop_back(){
        --_size;
    }

    void clear(){
        _size = 0;
    }

    void erase(size_t position){
        for(size_t i = position; i < _size - 1; ++i){
            data[i] = std::move(data[i+1]);
        }

        --_size;
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

private:
    void ensure_capacity(size_t new_capacity){
        if(_capacity == 0){
            _capacity = new_capacity;
            data = new T[_capacity];
        } else if(_capacity < new_capacity){
            _capacity= _capacity * 2;
            if(new_capacity > _capacity){
                _capacity = new_capacity;
            }

            auto new_data = new T[_capacity];
            std::move_n(new_data, data, _size);

            delete[] data;
            data = new_data;
        }
    }
};

} //end of namespace std

#endif
