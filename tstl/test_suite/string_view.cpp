//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <string.hpp>
#include <string_view.hpp>

#include "test.hpp"

namespace {

void test_small(){
    std::string s("asdf");
    auto sv = static_cast<std::string_view>(s);

    CHECK(!sv.empty(), "String mustn't be empty");
    CHECK(sv.size() == 4, "Invalid size");
    CHECK(sv == static_cast<std::string_view>(s), "Invalid content");
    CHECK(sv[0] == 'a', "invalid operator[]");
    CHECK(sv[1] == 's', "invalid operator[]");
    CHECK(sv[2] == 'd', "invalid operator[]");
    CHECK(sv[3] == 'f', "invalid operator[]");
    CHECK(*sv.begin() == 'a', "invalid begin()");

    sv.remove_prefix(1);

    CHECK(!sv.empty(), "String mustn't be empty");
    CHECK(sv.size() == 3, "Invalid size");
    CHECK(sv != static_cast<std::string_view>(s), "Invalid content");
    CHECK(sv[0] == 's', "invalid operator[]");
    CHECK(sv[1] == 'd', "invalid operator[]");
    CHECK(sv[2] == 'f', "invalid operator[]");
    CHECK(*sv.begin() == 's', "invalid operator[]");
}

void test_suffix(){
    std::string base("asdfpop");
    std::string s("asdfpop");
    auto sv = static_cast<std::string_view>(s);

    sv.remove_suffix(3);

    CHECK(!sv.empty(), "String mustn't be empty");
    CHECK(sv.size() == 4, "Invalid size");
    CHECK(sv != static_cast<std::string_view>(base), "Invalid content");
    CHECK(sv[0] == 'a', "invalid operator[]");
    CHECK(sv[1] == 's', "invalid operator[]");
    CHECK(sv[2] == 'd', "invalid operator[]");
    CHECK(sv[3] == 'f', "invalid operator[]");
    CHECK(*sv.begin() == 'a', "invalid begin()");
    CHECK(*(sv.end()-1) == 'f', "invalid end()");
}

void test_empty(){
    std::string_view s;

    CHECK(s.empty(), "String must be empty");
    CHECK(s.size() == 0, "Invalid size");
}

} //end of anonymous namespace

void string_view_tests(){
    test_small();
    test_suffix();
    test_empty();
}
