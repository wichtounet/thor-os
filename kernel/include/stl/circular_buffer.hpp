//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

template<typename T, size_t S>
struct circular_buffer {
private:
    static constexpr const size_t Size = S + 1;

    T buffer[Size];
    volatile size_t start;
    volatile size_t end;

public:
    circular_buffer() : start(0), end(0) {
        //Nothing to init
    }

    bool full() const {
        return (end + 1) % Size == start;
    }

    bool empty() const {
        return end == start;
    }

    bool push(T value){
        if(full()){
            return false;
        } else {
            buffer[end] = value;
            end = (end + 1) % Size;

            return true;
        }
    }

    T pop(){
        auto value = buffer[start];
        start = (start + 1) % Size;
        return value;
    }
};

#endif
