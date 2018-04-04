//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>

#include <circular_buffer.hpp>

#include "test.hpp"

namespace {

void base_test(){
    circular_buffer<size_t, 16> buffer;

    check(buffer.empty());
    check(!buffer.full());

    buffer.push(11);

    check(!buffer.empty());
    check(!buffer.full());
    check(buffer.top() == 11);

    buffer.push(22);

    check(buffer.top() == 11);

    check(buffer.pop() == 11);
    check(buffer.pop() == 22);
    check(buffer.empty());
}

void contains_test(){
    circular_buffer<size_t, 8> buffer;

    buffer.push(11);
    buffer.push(22);
    buffer.push(33);
    buffer.push(44);
    buffer.push(55);
    buffer.push(66);
    buffer.push(77);
    buffer.push(88);

    check(buffer.full());
    check(buffer.contains(88));
    check(buffer.contains(33));
    check(buffer.contains(11));
    check(!buffer.contains(13));

    buffer.replace(33, 13);
    check(!buffer.contains(33));
    check(buffer.contains(13));
}

} //end of anonymous namespace

void circular_buffer_tests(){
    base_test();
    contains_test();
}
