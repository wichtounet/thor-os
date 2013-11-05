#ifndef ARRAY_H
#define ARRAY_H

#include "thor.hpp"

template<typename T>
class unique_heap_array {
public:
    typedef T* pointer_type;

private:
    T* array;
    uint64_t _size;

public:
    unique_heap_array() : array(nullptr), _size(0) {}

    explicit unique_heap_array(T* a, uint64_t s) : array(a), _size(s) {}
    explicit unique_heap_array(uint64_t s) : _size(s) {
        array = reinterpret_cast<T*>(k_malloc(sizeof(T) * s));
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

    uint64_t size() const {
        return _size;
    }

    const T& operator[](uint64_t pos) const {
        return array[pos];
    }

    T& operator[](uint64_t pos){
        return array[pos];
    }

    pointer_type release(){
        pointer_type p = array;
        array = nullptr;
        return p;
    }

    void reset(pointer_type p = pointer_type()){
        if(array!= p){
            k_free(reinterpret_cast<uint64_t*>(array));
            array= nullptr;
        }
    }
};

#endif
