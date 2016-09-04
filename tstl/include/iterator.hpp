//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include <type_traits.hpp>
#include <utility.hpp>
#include <types.hpp>
#include <enable_if.hpp>

namespace std {

template<typename Iterator>
size_t distance(Iterator it, Iterator end){
    // For now, we only have random access iterator
    return end - it;
}

template< typename Container >
struct back_insert_iterator {
    using container_type = Container;
    using value_type = void;
    using difference_type = void;
    using reference = void;
    using const_reference = void;

    container_type& container;

    explicit back_insert_iterator(container_type& container) : container(container) {}

    back_insert_iterator& operator=(const typename container_type::value_type& value){
        container.push_back(value);

        return *this;
    }

    back_insert_iterator& operator=(typename container_type::value_type&& value){
        container.push_back(std::move(value));

        return *this;
    }

    back_insert_iterator& operator*(){
        return *this;
    }

    back_insert_iterator& operator++(){
        return *this;
    }

    back_insert_iterator& operator++(int){
        return *this;
    }
};

template< typename Container >
struct front_insert_iterator {
    using container_type = Container;
    using value_type = void;
    using difference_type = void;
    using reference = void;
    using const_reference = void;

    container_type& container;

    explicit front_insert_iterator(container_type& container) : container(container) {}

    front_insert_iterator& operator=(const typename container_type::value_type& value){
        container.push_front(value);

        return *this;
    }

    front_insert_iterator& operator=(typename container_type::value_type&& value){
        container.push_front(std::move(value));

        return *this;
    }

    front_insert_iterator& operator*(){
        return *this;
    }

    front_insert_iterator& operator++(){
        return *this;
    }

    front_insert_iterator& operator++(int){
        return *this;
    }
};

template <typename Container>
std::back_insert_iterator<Container> back_inserter(Container& c) {
    return std::back_insert_iterator<Container>(c);
}

template <typename Container>
std::front_insert_iterator<Container> front_inserter(Container& c) {
    return std::front_insert_iterator<Container>(c);
}

} //end of namespace std

#endif
