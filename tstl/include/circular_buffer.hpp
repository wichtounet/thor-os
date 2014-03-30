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

    volatile size_t tail;
    volatile size_t head;

public:
    circular_buffer() : tail(0), head(0) {
        //Nothing to init
    }

    bool full() const {
        return tail - head == S;
    }

    bool empty() const {
        return tail - head == 0;
    }

    void push(T value){
        buffer[tail % S] = value;
        ++tail;
    }

    T pop(){
        auto value = buffer[head % S];
        ++head;
        return value;
    }
};

#endif
