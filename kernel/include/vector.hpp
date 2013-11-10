#ifndef VECTOR_H
#define VECTOR_H

#include "types.hpp"

template<typename T>
class vector {
public:
    typedef T                       value_type;
    typedef value_type*             pointer_type;
    typedef uint64_t                size_type;
    typedef value_type*             iterator;
    typedef const value_type*       const_iterator;

private:
    T* data;
    uint64_t _size;
    uint64_t _capacity;

public:
    vector() : data(nullptr), _size(0), _capacity(0) {}
    explicit vector(uint64_t s) : data(new T[s]), _size(s), _capacity(s) {}

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
    }

    ~vector(){
        if(data){
            delete[] data;
        }
    }

    //Getters

    size_type size(){
        return _size;
    }

    //Modifiers

    void push_back(T& element){
        if(_capacity == _size){
            _capacity= _capacity == 0 ? 1 : _capacity * 2;
            auto new_data = new T[_capacity];

            for(size_type i = 0; i < _size; ++i){
                new_data[i] = data[i];
            }

            delete[] data;
            data = new_data;
        }

        data[_size++] = element;
    }

    //Iterators

    iterator begin(){
        return iterator(&data[0]);
    }

    const_iterator begin() const {
        return const_iterator(&data[0]);
    }

    iterator end(){
        return iterator(&data[_size]);
    }

    const_iterator end() const {
        return const_iterator(&data[_size]);
    }
};

#endif
