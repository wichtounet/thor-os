#ifndef UNIQUE_PTR_H
#define UNIQUE_PTR_H

#include "thor.hpp"

template <typename T>
class unique_ptr {
public:
    typedef T* pointer_type;
    typedef T element_type;

private:
    pointer_type pointer;

public:
    explicit unique_ptr(pointer_type p) : pointer(p){}

    unique_ptr() : pointer(pointer_type()) {}

    unique_ptr(unique_ptr&& u) : pointer(u.release()) {}
    unique_ptr& operator=(unique_ptr&& u){
        reset(u.release());
        return *this;
    }

    ~unique_ptr(){
        reset();
    }

    // Disable copy
    unique_ptr(const unique_ptr& rhs) = delete;
    unique_ptr& operator=(const unique_ptr& rhs) = delete;

    element_type& operator*() const {
        return *get();
    }

    pointer_type operator->() const {
        return get();
    }

    pointer_type get() const {
        return pointer;
    }

    pointer_type release(){
        pointer_type p = pointer;
        pointer = nullptr;
        return p;
    }

    void reset(pointer_type p = pointer_type()){
        if(pointer != p){
            k_free(reinterpret_cast<uint64_t*>(pointer));
            pointer = nullptr;
        }
    }
};

#endif
