//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STL_STACK_H
#define STL_STACK_H

#include "stl/vector.hpp"
#include "stl/types.hpp"

namespace std {

template<typename T, typename C = std::vector<T>>
struct stack {
private:
    C container;

public:
    bool empty() const {
        return size() == 0;
    }

    size_t size() const {
        return container.size();
    }

    void push(const T& value){
        container.push_back(value);
    }

    void pop(){
        container.pop_back();
    }

    T& top(){
        return container.back();
    }

    const T& top() const {
        return container.back();
    }
};

} //end of namespace std

#endif
