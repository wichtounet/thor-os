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

#include "test.hpp"

namespace {

void test_small(){
    std::string s("asdf");

    check(!s.empty(), "String mustn't be empty");
    check(s.size() == 4, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
    check(strcmp(s.c_str(), "asdf") == 0, "Invalid content");

    s += "1234";

    check(!s.empty(), "String mustn't be empty");
    check(s.size() == 8, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
    check(strcmp(s.c_str(), "asdf1234") == 0, "Invalid content");
}

void test_empty(){
    std::string s;

    check(s.empty(), "String must be empty");
    check(s.size() == 0, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
}

void test_limit(){
    std::string s("123456789abcdef");

    check(!s.empty(), "String mustn't be empty");
    check(s.size() == 15, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
    check(strcmp(s.c_str(), "123456789abcdef") == 0, "Invalid content");

    std::string s2("123456789abcdefg");

    check(!s2.empty(), "String mustn't be empty");
    check(s2.size() == 16, "Invalid size");
    check(s2.capacity() == 17, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789abcdefg") == 0, "Invalid content");
}

void test_grow(){
    std::string s("123456789abcdef");

    check(!s.empty(), "String mustn't be empty");
    check(s.size() == 15, "Invalid size");
    check(s.capacity() == 16, "Invalid capacity");
    check(strcmp(s.c_str(), "123456789abcdef") == 0, "Invalid content");

    s += 'g';

    check(!s.empty(), "String mustn't be empty");
    check_equals(s.size(), 16, "Invalid size");
    check_equals(s.capacity(), 32, "Invalid capacity");
    check(strcmp(s.c_str(), "123456789abcdefg") == 0, "Invalid content");

    s += 'g';

    check(!s.empty(), "String mustn't be empty");
    check_equals(s.size(), 17, "Invalid size");
    check_equals(s.capacity(), 32, "Invalid capacity");
    check(strcmp(s.c_str(), "123456789abcdefgg") == 0, "Invalid content");
}

void test_concat(){
    std::string s1("123456789");
    std::string s2("123456789");

    s1 += s2;

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 18, "Invalid size");
    check_equals(s1.capacity(), 32, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789123456789") == 0, "Invalid content");

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");

    std::string s3;
    s3 = s1 + s2;

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 18, "Invalid size");
    check_equals(s1.capacity(), 32, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789123456789") == 0, "Invalid content");

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");

    check(!s3.empty(), "String mustn't be empty");
    check_equals(s3.size(), 27, "Invalid size");
    check_equals(s3.capacity(), 32, "Invalid capacity");
    check(strcmp(s3.c_str(), "123456789123456789123456789") == 0, "Invalid content");
}

void test_concat_more(){
    std::string s1("123456789");
    std::string s2("123456789");

    s1 += s2;

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 18, "Invalid size");
    check_equals(s1.capacity(), 32, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789123456789") == 0, "Invalid content");

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");

    s1 += s2;

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 27, "Invalid size");
    check_equals(s1.capacity(), 32, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789123456789123456789") == 0, "Invalid content");

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");

    s1 += s2;

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 36, "Invalid size");
    check_equals(s1.capacity(), 64, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789123456789123456789123456789") == 0, "Invalid content");

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");
}

void test_move(){
    std::string s1("123456789");

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 9, "Invalid size");
    check_equals(s1.capacity(), 16, "Invalid capacity");
    check(strcmp(s1.c_str(), "123456789") == 0, "Invalid content");

    std::string s2(std::move(s1));

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 9, "Invalid size");
    check_equals(s2.capacity(), 16, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789") == 0, "Invalid content");

    check(s1.empty(), "Moved string must be empty");
    check_equals(s1.size(), 0, "Invalid size");

    std::string s3;
    s3 = std::move(s2);

    check(!s3.empty(), "String mustn't be empty");
    check_equals(s3.size(), 9, "Invalid size");
    check_equals(s3.capacity(), 16, "Invalid capacity");
    check(strcmp(s3.c_str(), "123456789") == 0, "Invalid content");

    check(s2.empty(), "Moved string must be empty");
    check_equals(s2.size(), 0, "Invalid size");

    s3 += std::string("123456789");

    check(!s3.empty(), "String mustn't be empty");
    check_equals(s3.size(), 18, "Invalid size");
    check_equals(s3.capacity(), 32, "Invalid capacity");
    check(strcmp(s3.c_str(), "123456789123456789") == 0, "Invalid content");

    std::string s4("asdf");

    s4 = std::move(s3);

    check(!s4.empty(), "String mustn't be empty");
    check_equals(s4.size(), 18, "Invalid size");
    check_equals(s4.capacity(), 32, "Invalid capacity");
    check(strcmp(s4.c_str(), "123456789123456789") == 0, "Invalid content");

    std::string s5("asdf");
    s4 = std::move(s5);

    check(!s4.empty(), "String mustn't be empty");
    check_equals(s4.size(), 4, "Invalid size");
    check_equals(s4.capacity(), 32, "Invalid capacity");
    check(strcmp(s4.c_str(), "asdf") == 0, "Invalid content");
}

void test_large(){
    char large_buffer[4097];

    char buffer[1025];
    for(int i = 0; i < 1024; ++i){
        buffer[i] = 'a' + (i % 9);
    }
    buffer[1024] = '\0';

    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < 1024; ++j){
            large_buffer[i * 1024 + j] = buffer[j];
        }
    }
    large_buffer[4096] = '\0';

    std::string s(buffer);

    check(!s.empty(), "String mustn't be empty");
    check_equals(s.size(), 1024, "Invalid size");
    check_equals(s.capacity(), 1025, "Invalid capacity");
    check(strcmp(s.c_str(), buffer) == 0, "Invalid content");

    s += buffer;
    s += buffer;
    s += buffer;

    check(!s.empty(), "String mustn't be empty");
    check_equals(s.size(), 4096, "Invalid size");
    check_equals(s.capacity(), 4100, "Invalid capacity");
    check(strcmp(s.c_str(), large_buffer) == 0, "Invalid content");
}

void test_reserve(){
    std::string s1("asdf");

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 4, "Invalid size");
    check_equals(s1.capacity(), 16, "Invalid capacity");
    check(strcmp(s1.c_str(), "asdf") == 0, "Invalid content");

    s1.reserve(1000);

    check(!s1.empty(), "String mustn't be empty");
    check_equals(s1.size(), 4, "Invalid size");
    check(s1.capacity() >= 1000, "Invalid capacity");
    check(strcmp(s1.c_str(), "asdf") == 0, "Invalid content");

    std::string s2("123456789012345678");

    s2.reserve(1000);

    check(!s2.empty(), "String mustn't be empty");
    check_equals(s2.size(), 18, "Invalid size");
    check(s2.capacity() >= 1000, "Invalid capacity");
    check(strcmp(s2.c_str(), "123456789012345678") == 0, "Invalid content");
}

void test_operators_short(){
    std::string short_string("asdfjkle");
    std::string empty_string;

    empty_string = short_string;

    check(!empty_string.empty(), "String mustn't be empty");
    check_equals(empty_string.size(), 8, "Invalid size");
    check(strcmp(empty_string.c_str(), "asdfjkle") == 0, "Invalid content");

    empty_string.clear();

    empty_string = std::move(short_string);

    check(!empty_string.empty(), "String mustn't be empty");
    check_equals(empty_string.size(), 8, "Invalid size");
    check(strcmp(empty_string.c_str(), "asdfjkle") == 0, "Invalid content");

    check_equals(short_string.size(), 0, "Invalid size");

    std::string first(empty_string);
    std::string last(std::move(empty_string));

    check(!first.empty(), "String mustn't be empty");
    check_equals(first.size(), 8, "Invalid size");
    check(strcmp(first.c_str(), "asdfjkle") == 0, "Invalid content");

    check(!last.empty(), "String mustn't be empty");
    check_equals(last.size(), 8, "Invalid size");
    check(strcmp(last.c_str(), "asdfjkle") == 0, "Invalid content");
}

void test_operators_long(){
    std::string short_string("asdfjkleasdfjkleasdfjkle");
    std::string empty_string;

    empty_string = short_string;

    check(!empty_string.empty(), "String mustn't be empty");
    check_equals(empty_string.size(), 24, "Invalid size");
    check(strcmp(empty_string.c_str(), "asdfjkleasdfjkleasdfjkle") == 0, "Invalid content");

    empty_string.clear();

    empty_string = std::move(short_string);

    check(!empty_string.empty(), "String mustn't be empty");
    check_equals(empty_string.size(), 24, "Invalid size");
    check(strcmp(empty_string.c_str(), "asdfjkleasdfjkleasdfjkle") == 0, "Invalid content");

    check_equals(short_string.size(), 0, "Invalid size");

    std::string first(empty_string);
    std::string last(std::move(empty_string));

    check(!first.empty(), "String mustn't be empty");
    check_equals(first.size(), 24, "Invalid size");
    check(strcmp(first.c_str(), "asdfjkleasdfjkleasdfjkle") == 0, "Invalid content");

    check(!last.empty(), "String mustn't be empty");
    check_equals(last.size(), 24, "Invalid size");
    check(strcmp(last.c_str(), "asdfjkleasdfjkleasdfjkle") == 0, "Invalid content");
}

void test_operators_short_to_long(){
    std::string short_string("asdfjkleasdfjkleasdfjkle");
    std::string empty_string("asdf");;

    empty_string = short_string;

    check(!empty_string.empty(), "String mustn't be empty");
    check_equals(empty_string.size(), 24, "Invalid size");
    check(strcmp(empty_string.c_str(), "asdfjkleasdfjkleasdfjkle") == 0, "Invalid content");

    empty_string.clear();

    empty_string = std::move(short_string);

    check(!empty_string.empty(), "String mustn't be empty");
    check_equals(empty_string.size(), 24, "Invalid size");
    check(strcmp(empty_string.c_str(), "asdfjkleasdfjkleasdfjkle") == 0, "Invalid content");

    check_equals(short_string.size(), 0, "Invalid size");

    std::string first(empty_string);
    std::string last(std::move(empty_string));

    check(!first.empty(), "String mustn't be empty");
    check_equals(first.size(), 24, "Invalid size");
    check(strcmp(first.c_str(), "asdfjkleasdfjkleasdfjkle") == 0, "Invalid content");

    check(!last.empty(), "String mustn't be empty");
    check_equals(last.size(), 24, "Invalid size");
    check(strcmp(last.c_str(), "asdfjkleasdfjkleasdfjkle") == 0, "Invalid content");
}

void test_operators_long_to_short(){
    std::string short_string("asdf");
    std::string empty_string("asdfjkleasdfjkleasdfjkle");;

    empty_string = short_string;

    check(!empty_string.empty(), "String mustn't be empty");
    check_equals(empty_string.size(), 4, "Invalid size");
    check(strcmp(empty_string.c_str(), "asdf") == 0, "Invalid content");

    empty_string.clear();

    empty_string = std::move(short_string);

    check(!empty_string.empty(), "String mustn't be empty");
    check_equals(empty_string.size(), 4, "Invalid size");
    check(strcmp(empty_string.c_str(), "asdf") == 0, "Invalid content");

    check_equals(short_string.size(), 0, "Invalid size");

    std::string first(empty_string);
    std::string last(std::move(empty_string));

    check(!first.empty(), "String mustn't be empty");
    check_equals(first.size(), 4, "Invalid size");
    check(strcmp(first.c_str(), "asdf") == 0, "Invalid content");

    check(!last.empty(), "String mustn't be empty");
    check_equals(last.size(), 4, "Invalid size");
    check(strcmp(last.c_str(), "asdf") == 0, "Invalid content");
}

void test_compare(){
    std::string a = "bcd";
    std::string b = "bcde";
    std::string c = "abcd";
    std::string d = "abcde";
    std::string e = "bcd";

    CHECK(a == a, "Invalid operator==");
    CHECK(a == e, "Invalid operator==");
    CHECK(e == a, "Invalid operator==");

    CHECK(a != b, "Invalid operator!=");
    CHECK(a != c, "Invalid operator!=");
    CHECK(a != d, "Invalid operator!=");

    CHECK(a.compare(a) == 0, "Invalid std::string::compare");
    CHECK(a.compare(b) == -1, "Invalid std::string::compare");
    CHECK(a.compare(c) == 1, "Invalid std::string::compare");
    CHECK(a.compare(d) == 1, "Invalid std::string::compare");
    CHECK(a.compare(e) == 0, "Invalid std::string::compare");

    CHECK(b.compare(a) == 1, "Invalid std::string::compare");
    CHECK(b.compare(b) == 0, "Invalid std::string::compare");
    CHECK(b.compare(c) == 1, "Invalid std::string::compare");
    CHECK(b.compare(d) == 1, "Invalid std::string::compare");
    CHECK(b.compare(e) == 1, "Invalid std::string::compare");

    CHECK(c.compare(a) == -1, "Invalid std::string::compare");
    CHECK(c.compare(b) == -1, "Invalid std::string::compare");
    CHECK(c.compare(c) == 0, "Invalid std::string::compare");
    CHECK(c.compare(d) == -1, "Invalid std::string::compare");
    CHECK(c.compare(e) == -1, "Invalid std::string::compare");

    CHECK(d.compare(a) == -1, "Invalid std::string::compare");
    CHECK(d.compare(b) == -1, "Invalid std::string::compare");
    CHECK(d.compare(c) == 1, "Invalid std::string::compare");
    CHECK(d.compare(d) == 0, "Invalid std::string::compare");
    CHECK(d.compare(e) == -1, "Invalid std::string::compare");

    CHECK(e.compare(a) == 0, "Invalid std::string::compare");
    CHECK(e.compare(b) == -1, "Invalid std::string::compare");
    CHECK(e.compare(c) == 1, "Invalid std::string::compare");
    CHECK(e.compare(d) == 1, "Invalid std::string::compare");
    CHECK(e.compare(e) == 0, "Invalid std::string::compare");
}

void test_assign_sv(){
    std::string a = "bcd";
    std::string b = "abcde";
    std::string_view sb = b;
    std::string c = "def";

    a = sb;

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 5, "Invalid size");
    CHECK(strcmp(a.c_str(), "abcde") == 0, "Invalid content");

    a = static_cast<std::string_view>(c);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 3, "Invalid size");
    CHECK(strcmp(a.c_str(), "def") == 0, "Invalid content");
}

void test_append_1(){
    std::string a = "bcd";
    std::string b = "efg";
    std::string_view sb = b;

    a.append(sb);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 6, "Invalid size");
    CHECK(strcmp(a.c_str(), "bcdefg") == 0, "Invalid content");

    a.append(b);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 9, "Invalid size");
    CHECK(strcmp(a.c_str(), "bcdefgefg") == 0, "Invalid content");
}

void test_append_2(){
    std::string a = "bcd";
    std::string b = "efg";
    std::string_view sb = b;

    a += sb;

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 6, "Invalid size");
    CHECK(strcmp(a.c_str(), "bcdefg") == 0, "Invalid content");

    a += b;

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 9, "Invalid size");
    CHECK(strcmp(a.c_str(), "bcdefgefg") == 0, "Invalid content");
}

void test_append_3(){
    std::string a = "bcd";
    std::string b = "aefga";
    std::string_view sb = b;

    a.append(sb.begin() + 1, sb.end() - 1);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 6, "Invalid size");
    CHECK(strcmp(a.c_str(), "bcdefg") == 0, "Invalid content");

    a.append(b.begin() + 1, b.end() - 1);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 9, "Invalid size");
    CHECK(strcmp(a.c_str(), "bcdefgefg") == 0, "Invalid content");
}

void test_assign_1(){
    std::string a = "bcd";
    std::string b = "efg";
    std::string_view sb = b;

    a.assign(sb);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 3, "Invalid size");
    CHECK(strcmp(a.c_str(), "efg") == 0, "Invalid content");

    a = "";
    a.assign(b);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 3, "Invalid size");
    CHECK(strcmp(a.c_str(), "efg") == 0, "Invalid content");
}

void test_assign_2(){
    std::string a = "bcd";
    std::string b = "aefga";
    std::string_view sb = b;

    a.assign(sb.begin() + 1, sb.end() - 1);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 3, "Invalid size");
    CHECK(strcmp(a.c_str(), "efg") == 0, "Invalid content");

    a = "";
    a.assign(b.begin() + 1, b.end() - 1);

    CHECK(!a.empty(), "String mustn't be empty");
    CHECK(a.size() == 3, "Invalid size");
    CHECK(strcmp(a.c_str(), "efg") == 0, "Invalid content");
}

} //end of anonymous namespace

void string_tests(){
    test_small();
    test_empty();
    test_limit();
    test_grow();
    test_concat();
    test_concat_more();
    test_move();
    test_large();
    test_reserve();
    test_operators_short();
    test_operators_long();
    test_operators_short_to_long();
    test_operators_long_to_short();
    test_compare();
    test_assign_sv();
    test_assign_1();
    test_assign_2();
    test_append_1();
    test_append_2();
    test_append_3();
}
