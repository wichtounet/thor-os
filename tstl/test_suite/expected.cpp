//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>

#include <expected.hpp>

#include "test.hpp"

namespace {

struct kiss {
    int* ref;
    kiss(){}
    kiss(int* ref) : ref(ref) {}
    ~kiss(){
        ++(*ref);
    }
};

void test_destructor() {
    int counter = 0;
    kiss k(&counter);

    {
        auto e = std::make_expected<kiss>(k);
    }

    check_equals(counter, 1, "destruct: Invalid destructors");
}

} //end of anonymous namespace

void expected_tests(){
    test_destructor();
}
