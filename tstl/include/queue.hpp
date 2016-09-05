//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STL_QUEUE_H
#define STL_QUEUE_H

#include <list.hpp>
#include <types.hpp>

namespace std {

template<typename T, typename C = std::list<T>>
struct queue {
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
        container.pop_front();
    }

    T& top(){
        return container.front();
    }

    const T& top() const {
        return container.front();
    }
};

} //end of namespace std

#endif
