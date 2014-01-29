//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LIST_H
#define LIST_H

#include "stl/types.hpp"

namespace std {

template<typename T>
struct list_node;

template<typename T>
class list {
public:
    typedef T                       value_type;
    typedef value_type*             pointer_type;
    typedef size_t                  size_type;
    typedef list_node<T>            node_type;

private:
    size_t _size;
    node_type* head;
    node_type* tail;

public:
    list() : _size(0), head(nullptr), tail(nullptr) {
        //Nothing else to init
    }

    // Disable copy for now
    list(const list& rhs) = delete;
    list& operator=(const list& rhs) = delete;

    // Disable move for now
    list(list&& rhs) = delete;
    list& operator=(list&& rhs) = delete;

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size;
    }

    void push_front(const value_type& value){
        if(_size == 0){
            head = new node_type(value, nullptr, nullptr);
            tail = head;
        } else {
            auto node = new node_type(value, head, nullptr);
            head->prev = node;
            head = node;
        }

        ++_size;
    }

    void push_back(const value_type& value){
        if(_size == 0){
            head = new node_type(value, nullptr, nullptr);
            tail = head;
        } else {
            auto node = new node_type(value, nullptr, tail);
            tail->next = node;
            tail = node;
        }

        ++_size;
    }

    void pop_front(){
        auto old = head;

        if(_size == 1){
            tail = head = nullptr;
        } else {
            head = head->next;
            head->prev = nullptr;
        }

        delete old;

        --_size;
    }

    void pop_back(){
        auto old = tail;

        if(_size == 1){
            tail = head = nullptr;
        } else {
            tail = tail->prev;
            tail->next = nullptr;
        }

        delete old;

        --_size;
    }

    const T& front() const {
        return head->value;
    }

    T& front(){
        return head->value;
    }

    const T& back() const {
        return tail->value;
    }

    T& back(){
        return tail->value;
    }

    //TODO
};

template<typename T>
struct list_node {
    T value;
    list_node<T>* next;
    list_node<T>* prev;

    list_node(const T& v, list_node<T>* n, list_node<T>* p) : value(v), next(n), prev(p) {
        //Nothing else to init
    }
};

} //end of namespace std

#endif
