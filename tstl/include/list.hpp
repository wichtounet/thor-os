//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef LIST_H
#define LIST_H

#include <initializer_list.hpp>

#include <types.hpp>
#include <type_traits.hpp>
#include <iterator.hpp>

namespace std {

template<typename T>
struct list_node;

template<typename T>
struct list;

template <typename T, typename V>
struct list_iterator {
    using value_type      = V;
    using node_type       = list_node<T>;
    using reference       = value_type&;
    using pointer         = value_type*;
    using difference_type = void;

    list_iterator(node_type* current) : current(current){
        //Nothing else to init
    }

    value_type& operator*(){
        return current->value;
    }

    const value_type& operator*() const {
        return current->value;
    }

    list_iterator& operator++(){
        current = current->next;
        return *this;
    }

    list_iterator operator++(int){
        list_iterator v = *this;
        current = current->next;
        return v;
    }

    list_iterator& operator--(){
        current = current->prev;
        return *this;
    }

    list_iterator operator--(int){
        list_iterator v = *this;
        current = current->prev;
        return v;
    }

    bool operator==(const list_iterator& rhs){
        return current == rhs.current;
    }

    bool operator!=(const list_iterator& rhs){
        return !(*this == rhs);
    }

    friend struct list<T>;

private:
    node_type* current;
};

/*!
 * \brief A doubly-linked list implementation.
 *
 * This container should almost never be preferred over vector. The only time when list is better than vector
 * is when the data is very expensive to copy or it is necessary to have stable references.
 */
template<typename T>
struct list {
    using value_type             = T;                                                            ///7< The value type of the container
    using pointer_type           = value_type*;                                                  ///< The pointer type of the container
    using reference_type         = value_type&;                                                  ///< The pointer type of the container
    using const_reference_type   = const value_type&;                                            ///< The pointer type of the container
    using size_type              = size_t;                                                       ///< The size type of the container
    using node_type              = list_node<T>;                                                 ///< The type of nodes
    using iterator               = list_iterator<T, T>;                                          ///< The iterator type
    using const_iterator         = list_iterator<T, std::add_const_t<T>>;                        ///< The const iterator type
    using reverse_iterator       = std::reverse_iterator<list_iterator<T, T>>;                   ///< The reverse iterator type
    using const_reverse_iterator = std::reverse_iterator<list_iterator<T, std::add_const_t<T>>>; ///< The const reverse iterator type

    /*!
     * \brief Construct a ne empty list
     */
    list() : _size(0), head(nullptr), tail(nullptr) {
        //Nothing else to init
    }

    /*!
     * \brief Destructs a list
     */
    ~list(){
        clear();
    }

    // Disable copy for now
    list(const list& rhs) = delete;
    list& operator=(const list& rhs) = delete;

    //Allow move
    list(list&& rhs) : _size(rhs._size), head(rhs.head), tail(rhs.tail){
        rhs._size = 0;
        rhs.head = nullptr;
        rhs.tail = nullptr;
    }

    list(initializer_list<T> values) : list() {
        for(auto& v : values){
            push_back(v);
        }
    }

    list& operator=(list&& rhs){
        if(size() > 0){
            clear();
        }

        _size = rhs._size;
        head = rhs.head;
        tail = rhs.tail;

        rhs._size = 0;
        rhs.head = nullptr;
        rhs.tail = nullptr;

        return *this;
    }

    /*!
     * \brief Returns the size of the container
     */
    size_t size() const {
        return _size;
    }

    /*!
     * \brief Indicates if the list is empty
     */
    bool empty() const {
        return !_size;
    }

    /*!
     * \brief Empty the list
     */
    void clear(){
        while(!empty()){
            pop_back();
        }
    }

    /*!
     * \brief Add a new element at the front of the list
     */
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

    /*!
     * \brief Add a new element at the back of the list
     */
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

    template<typename... Args>
    value_type& emplace_front(Args&&... args){
        if(_size == 0){
            head = new node_type(nullptr, nullptr, std::forward<Args>(args)...);
            tail = head;
        } else {
            auto node = new node_type(head, nullptr, std::forward<Args>(args)...);
            head->prev = node;
            head = node;
        }

        ++_size;

        return head->value;
    }

    template<typename... Args>
    value_type& emplace_back(Args&&... args){
        if(_size == 0){
            head = new node_type(nullptr, nullptr, std::forward<Args>(args)...);
            tail = head;
        } else {
            auto node = new node_type(nullptr, tail, std::forward<Args>(args)...);
            tail->next = node;
            tail = node;
        }

        ++_size;

        return tail->value;
    }

    /*!
     * \brief Removes the element at the front of the list
     */
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

    /*!
     * \brief Removes the element at the back of the list
     */
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

private:
    iterator erase_node(node_type* node){
        if(!node){
            return end();
        }

        if(node->prev){
            node->prev->next = node->next;
        }

        if(node->next){
            node->next->prev = node->prev;
        }

        if(head == node){
            head = node->next;
        }

        if(tail == node){
            tail = node->prev;
        }

        auto next = node->next;

        delete node;

        --_size;

        return iterator(next);
    }

public:
    iterator erase(iterator it){
        return erase_node(it.current);
    }

    iterator erase(const_iterator it){
        return erase_node(it.current);
    }

    iterator erase(iterator it, iterator last){
        while(it != last){
            it = erase_node(it.current);
        }

        return last;
    }

    iterator erase(const_iterator it, const_iterator last){
        while(it != last){
            it = erase_node(it.current);
        }

        return iterator(last.current);
    }

    // Element access

    /*!
     * \brief Returns a reference to the element at the front of the list
     */
    T& front(){
        return head->value;
    }

    /*!
     * \brief Returns a const reference to the element at the front of the list
     */
    const T& front() const {
        return head->value;
    }

    /*!
     * \brief Returns a reference to the element at the back of the list
     */
    T& back(){
        return tail->value;
    }

    /*!
     * \brief Returns a const reference to the element at the back of the list
     */
    const T& back() const {
        return tail->value;
    }

    // Iterators

    /*!
     * \brief Returns an iterator pointing to the first element of the list
     */
    iterator begin(){
        return iterator(head);
    }

    /*!
     * \brief Returns a const iterator pointing to the first element of the list
     */
    const iterator begin() const {
        return const_iterator(head);
    }

    /*!
     * \brief Returns an iterator pointing to the past-the-end element of the list
     */
    iterator end(){
        return iterator(nullptr);
    }

    /*!
     * \brief Returns a const iterator pointing to the past-the-end element of the list
     */
    const_iterator end() const {
        return const_iterator(nullptr);
    }

    reverse_iterator rbegin(){
        return reverse_iterator(tail);
    }

    constexpr const_reverse_iterator rbegin() const {
        return const_iterator(tail);
    }

    reverse_iterator rend(){
        return reverse_iterator(nullptr);
    }

    constexpr const_reverse_iterator rend() const {
        return const_reverse_iterator(nullptr);
    }

private:
    size_t _size; ///< The size of the list
    node_type* head; ///< The front node of the list
    node_type* tail; ///< The tail node of the list
};

template<typename T>
struct list_node {
    list_node(const T& v, list_node<T>* n, list_node<T>* p) : value(v), next(n), prev(p) {
        //Nothing else to init
    }

    template<typename... Args>
    list_node(list_node<T>* n, list_node<T>* p, Args&&... args) : value(std::forward<Args>(args)...), next(n), prev(p) {
        //Nothing else to init
    }

    friend struct list<T>;
    friend struct list_iterator<T, T>;
    friend struct list_iterator<T, std::add_const_t<T>>;

private:
    T value;
    list_node<T>* next;
    list_node<T>* prev;
};

} //end of namespace std

#endif
