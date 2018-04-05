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

void test_compare(){
    std::string sa = "bcd";
    std::string sb = "bcde";
    std::string sc = "abcd";
    std::string sd = "abcde";
    std::string se = "bcd";

    std::string_view a = sa;
    std::string_view b = sb;
    std::string_view c = sc;
    std::string_view d = sd;
    std::string_view e = se;

    CHECK(a == a, "Invalid operator==");
    CHECK(a == e, "Invalid operator==");
    CHECK(e == a, "Invalid operator==");

    CHECK(a != b, "Invalid operator!=");
    CHECK(a != c, "Invalid operator!=");
    CHECK(a != d, "Invalid operator!=");

    CHECK(a.compare(a) == 0, "Invalid std::string_view::compare");
    CHECK(a.compare(b) == -1, "Invalid std::string_view::compare");
    CHECK(a.compare(c) == 1, "Invalid std::string_view::compare");
    CHECK(a.compare(d) == 1, "Invalid std::string_view::compare");
    CHECK(a.compare(e) == 0, "Invalid std::string_view::compare");

    CHECK(b.compare(a) == 1, "Invalid std::string_view::compare");
    CHECK(b.compare(b) == 0, "Invalid std::string_view::compare");
    CHECK(b.compare(c) == 1, "Invalid std::string_view::compare");
    CHECK(b.compare(d) == 1, "Invalid std::string_view::compare");
    CHECK(b.compare(e) == 1, "Invalid std::string_view::compare");

    CHECK(c.compare(a) == -1, "Invalid std::string_view::compare");
    CHECK(c.compare(b) == -1, "Invalid std::string_view::compare");
    CHECK(c.compare(c) == 0, "Invalid std::string_view::compare");
    CHECK(c.compare(d) == -1, "Invalid std::string_view::compare");
    CHECK(c.compare(e) == -1, "Invalid std::string_view::compare");

    CHECK(d.compare(a) == -1, "Invalid std::string_view::compare");
    CHECK(d.compare(b) == -1, "Invalid std::string_view::compare");
    CHECK(d.compare(c) == 1, "Invalid std::string_view::compare");
    CHECK(d.compare(d) == 0, "Invalid std::string_view::compare");
    CHECK(d.compare(e) == -1, "Invalid std::string_view::compare");

    CHECK(e.compare(a) == 0, "Invalid std::string_view::compare");
    CHECK(e.compare(b) == -1, "Invalid std::string_view::compare");
    CHECK(e.compare(c) == 1, "Invalid std::string_view::compare");
    CHECK(e.compare(d) == 1, "Invalid std::string_view::compare");
    CHECK(e.compare(e) == 0, "Invalid std::string_view::compare");
}

void test_mixed_compare(){
    std::string sa = "bcd";
    std::string sb = "bcde";
    std::string sc = "abcd";
    std::string sd = "abcde";
    std::string se = "bcd";

    std::string_view a = sa;
    std::string_view b = sb;
    std::string_view c = sc;
    std::string_view d = sd;
    std::string_view e = se;

    CHECK(a == sa, "Invalid operator==");
    CHECK(a == se, "Invalid operator==");
    CHECK(e == sa, "Invalid operator==");

    CHECK(sa == a, "Invalid operator==");
    CHECK(sa == e, "Invalid operator==");
    CHECK(se == a, "Invalid operator==");

    CHECK(a != sb, "Invalid operator!=");
    CHECK(a != sc, "Invalid operator!=");
    CHECK(a != sd, "Invalid operator!=");

    CHECK(sa != b, "Invalid operator!=");
    CHECK(sa != c, "Invalid operator!=");
    CHECK(sa != d, "Invalid operator!=");
}

} //end of anonymous namespace

void string_view_tests(){
    test_small();
    test_suffix();
    test_empty();
    test_compare();
    test_mixed_compare();
}
