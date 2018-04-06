//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "vfs/path.hpp"

#include "test.hpp"

namespace {

void test_empty(){
    path p;

    CHECK_DIRECT(p.empty());
    CHECK_DIRECT(!p.is_valid());
    CHECK_EQUALS_DIRECT(p.size(), 0);
}

void test_root(){
    path p("/");

    CHECK_DIRECT(!p.empty());
    CHECK_DIRECT(p.is_valid());
    CHECK_DIRECT(p.is_absolute());
    CHECK_DIRECT(p.is_root());
    CHECK_DIRECT(!p.is_sub_root());
    CHECK_EQUALS_DIRECT(p.size(), 1);
    CHECK_DIRECT(p.root_name() == "/");
    CHECK_DIRECT(p.base_name() == "/");
    CHECK_DIRECT(p.name(0) == "/");
}

void test_path_0(){
    path p1("/a1");
    path p2("/a1/");

    CHECK_DIRECT(!p1.empty());
    CHECK_DIRECT(p1.is_valid());
    CHECK_DIRECT(p1.is_absolute());
    CHECK_DIRECT(p1.is_sub_root());
    CHECK_EQUALS_DIRECT(p1.size(), 2);
    CHECK_DIRECT(p1.root_name() == "/");
    CHECK_DIRECT(p1.base_name() == "a1");
    CHECK_DIRECT(p1.sub_root_name() == "a1");
    CHECK_DIRECT(p1.name(0) == "/");
    CHECK_DIRECT(p1.name(1) == "a1");

    CHECK_DIRECT(!p2.empty());
    CHECK_DIRECT(p2.is_valid());
    CHECK_DIRECT(p2.is_absolute());
    CHECK_DIRECT(p2.is_sub_root());
    CHECK_EQUALS_DIRECT(p2.size(), 2);
    CHECK_DIRECT(p2.root_name() == "/");
    CHECK_DIRECT(p2.base_name() == "a1");
    CHECK_DIRECT(p2.sub_root_name() == "a1");
    CHECK_DIRECT(p2.name(0) == "/");
    CHECK_DIRECT(p2.name(1) == "a1");

    CHECK_DIRECT(p1 == p1);
    CHECK_DIRECT(p1 == p2);
    CHECK_DIRECT(p2 == p1);
}

void test_path_1(){
    path p1("a1");
    path p2("a1/");

    CHECK_DIRECT(!p1.empty());
    CHECK_DIRECT(p1.is_valid());
    CHECK_DIRECT(!p1.is_absolute());
    CHECK_DIRECT(!p1.is_sub_root());
    CHECK_EQUALS_DIRECT(p1.size(), 1);
    CHECK_DIRECT(p1.root_name() == "a1");
    CHECK_DIRECT(p1.base_name() == "a1");
    CHECK_DIRECT(p1.name(0) == "a1");

    CHECK_DIRECT(!p2.empty());
    CHECK_DIRECT(p2.is_valid());
    CHECK_DIRECT(!p2.is_absolute());
    CHECK_DIRECT(!p2.is_sub_root());
    CHECK_EQUALS_DIRECT(p2.size(), 1);
    CHECK_DIRECT(p2.root_name() == "a1");
    CHECK_DIRECT(p2.base_name() == "a1");
    CHECK_DIRECT(p2.name(0) == "a1");

    CHECK_DIRECT(p1 == p1);
    CHECK_DIRECT(p1 == p2);
    CHECK_DIRECT(p2 == p1);
}

void test_path_2(){
    path p1("/a1/b2/c");
    path p2("/a1/b2/c/");

    CHECK_DIRECT(!p1.empty());
    CHECK_DIRECT(p1.is_valid());
    CHECK_DIRECT(p1.is_absolute());
    CHECK_DIRECT(!p1.is_sub_root());
    CHECK_EQUALS_DIRECT(p1.size(), 4);
    CHECK_DIRECT(p1.root_name() == "/");
    CHECK_DIRECT(p1.base_name() == "c");
    CHECK_DIRECT(p1.sub_root_name() == "a1");
    CHECK_DIRECT(p1.name(0) == "/");
    CHECK_DIRECT(p1.name(1) == "a1");
    CHECK_DIRECT(p1.name(2) == "b2");
    CHECK_DIRECT(p1.name(3) == "c");

    //printf("%.*s \n", int(p1.name(1).size()), p1.name(1).data());

    CHECK_DIRECT(!p2.empty());
    CHECK_DIRECT(p2.is_valid());
    CHECK_DIRECT(p2.is_absolute());
    CHECK_DIRECT(!p2.is_sub_root());
    CHECK_EQUALS_DIRECT(p2.size(), 4);
    CHECK_DIRECT(p2.root_name() == "/");
    CHECK_DIRECT(p2.base_name() == "c");
    CHECK_DIRECT(p2.sub_root_name() == "a1");
    CHECK_DIRECT(p2.name(0) == "/");
    CHECK_DIRECT(p2.name(1) == "a1");
    CHECK_DIRECT(p2.name(2) == "b2");
    CHECK_DIRECT(p2.name(3) == "c");

    CHECK_DIRECT(p1 == p1);
    CHECK_DIRECT(p1 == p2);
    CHECK_DIRECT(p2 == p1);
}

void test_path_3(){
    path p1("a1/b2/c");
    path p2("a1/b2/c/");

    CHECK_DIRECT(!p1.empty());
    CHECK_DIRECT(p1.is_valid());
    CHECK_DIRECT(!p1.is_absolute());
    CHECK_DIRECT(!p1.is_sub_root());
    CHECK_EQUALS_DIRECT(p1.size(), 3);
    CHECK_DIRECT(p1.root_name() == "a1");
    CHECK_DIRECT(p1.base_name() == "c");
    CHECK_DIRECT(p1.name(0) == "a1");
    CHECK_DIRECT(p1.name(1) == "b2");
    CHECK_DIRECT(p1.name(2) == "c");

    CHECK_DIRECT(!p2.empty());
    CHECK_DIRECT(p2.is_valid());
    CHECK_DIRECT(!p2.is_absolute());
    CHECK_DIRECT(!p2.is_sub_root());
    CHECK_EQUALS_DIRECT(p2.size(), 3);
    CHECK_DIRECT(p2.root_name() == "a1");
    CHECK_DIRECT(p2.base_name() == "c");
    CHECK_DIRECT(p2.name(0) == "a1");
    CHECK_DIRECT(p2.name(1) == "b2");
    CHECK_DIRECT(p2.name(2) == "c");

    CHECK_DIRECT(p1 == p1);
    CHECK_DIRECT(p1 == p2);
    CHECK_DIRECT(p2 == p1);
}

void test_invalidate(){
    path p1("/a1/b2/c");
    path p2("a1/b2/c/");

    p1.invalidate();
    p2.invalidate();

    CHECK_DIRECT(!p1.is_valid());
    CHECK_DIRECT(!p2.is_valid());
}

void test_not_equals(){
    path p1("/a1/b2/c");
    path p2("a1/b2/c/");

    CHECK_DIRECT(p1 != p2);
    CHECK_DIRECT(p2 != p1);

    CHECK_DIRECT(!(p1 == p2));
    CHECK_DIRECT(!(p2 == p1));

    CHECK_DIRECT(!(p1 == "/a1/b3"));
    CHECK_DIRECT(!(p1 == "/"));
    CHECK_DIRECT(!(p1 == "/a1/b2/c1"));
}

void test_concat_0(){
    auto p1 = path("/a1") / path("b2") / path("c");
    auto p2 = path("/a1") / "b2" / "c";
    auto p3 = path("/a1/b2") / path("c");
    auto p4 = path("/") / path("a1/b2") / "c";
    auto p5 = path("/a1") / "b2/c";
    auto p6 = path() / path("/") / path("a1") / path("b2/c");

    CHECK_DIRECT(!p1.empty());
    CHECK_DIRECT(p1.is_valid());
    CHECK_DIRECT(p1.is_absolute());
    CHECK_DIRECT(!p1.is_sub_root());
    CHECK_EQUALS_DIRECT(p1.size(), 4);
    CHECK_DIRECT(p1.root_name() == "/");
    CHECK_DIRECT(p1.base_name() == "c");
    CHECK_DIRECT(p1.sub_root_name() == "a1");
    CHECK_DIRECT(p1.name(0) == "/");
    CHECK_DIRECT(p1.name(1) == "a1");
    CHECK_DIRECT(p1.name(2) == "b2");
    CHECK_DIRECT(p1.name(3) == "c");

    CHECK_DIRECT(!p2.empty());
    CHECK_DIRECT(p2.is_valid());
    CHECK_DIRECT(p2.is_absolute());
    CHECK_DIRECT(!p2.is_sub_root());
    CHECK_EQUALS_DIRECT(p2.size(), 4);
    CHECK_DIRECT(p2.root_name() == "/");
    CHECK_DIRECT(p2.base_name() == "c");
    CHECK_DIRECT(p2.sub_root_name() == "a1");
    CHECK_DIRECT(p2.name(0) == "/");
    CHECK_DIRECT(p2.name(1) == "a1");
    CHECK_DIRECT(p2.name(2) == "b2");
    CHECK_DIRECT(p2.name(3) == "c");

    CHECK_DIRECT(!p3.empty());
    CHECK_DIRECT(p3.is_valid());
    CHECK_DIRECT(p3.is_absolute());
    CHECK_DIRECT(!p3.is_sub_root());
    CHECK_EQUALS_DIRECT(p3.size(), 4);
    CHECK_DIRECT(p3.root_name() == "/");
    CHECK_DIRECT(p3.base_name() == "c");
    CHECK_DIRECT(p3.sub_root_name() == "a1");
    CHECK_DIRECT(p3.name(0) == "/");
    CHECK_DIRECT(p3.name(1) == "a1");
    CHECK_DIRECT(p3.name(2) == "b2");
    CHECK_DIRECT(p3.name(3) == "c");

    CHECK_DIRECT(!p4.empty());
    CHECK_DIRECT(p4.is_valid());
    CHECK_DIRECT(p4.is_absolute());
    CHECK_DIRECT(!p4.is_sub_root());
    CHECK_EQUALS_DIRECT(p4.size(), 4);
    CHECK_DIRECT(p4.root_name() == "/");
    CHECK_DIRECT(p4.base_name() == "c");
    CHECK_DIRECT(p4.sub_root_name() == "a1");
    CHECK_DIRECT(p4.name(0) == "/");
    CHECK_DIRECT(p4.name(1) == "a1");
    CHECK_DIRECT(p4.name(2) == "b2");
    CHECK_DIRECT(p4.name(3) == "c");

    CHECK_DIRECT(!p5.empty());
    CHECK_DIRECT(p5.is_valid());
    CHECK_DIRECT(p5.is_absolute());
    CHECK_DIRECT(!p5.is_sub_root());
    CHECK_EQUALS_DIRECT(p5.size(), 4);
    CHECK_DIRECT(p5.root_name() == "/");
    CHECK_DIRECT(p5.base_name() == "c");
    CHECK_DIRECT(p5.sub_root_name() == "a1");
    CHECK_DIRECT(p5.name(0) == "/");
    CHECK_DIRECT(p5.name(1) == "a1");
    CHECK_DIRECT(p5.name(2) == "b2");
    CHECK_DIRECT(p5.name(3) == "c");

    CHECK_DIRECT(!p6.empty());
    CHECK_DIRECT(p6.is_valid());
    CHECK_DIRECT(p6.is_absolute());
    CHECK_DIRECT(!p6.is_sub_root());
    CHECK_EQUALS_DIRECT(p6.size(), 4);
    CHECK_DIRECT(p6.root_name() == "/");
    CHECK_DIRECT(p6.base_name() == "c");
    CHECK_DIRECT(p6.sub_root_name() == "a1");
    CHECK_DIRECT(p6.name(0) == "/");
    CHECK_DIRECT(p6.name(1) == "a1");
    CHECK_DIRECT(p6.name(2) == "b2");
    CHECK_DIRECT(p6.name(3) == "c");
}

void test_concat_1(){
    auto p1 = path("a1") / path("b2") / path("c");
    auto p2 = path("a1") / "b2" / "c";
    auto p3 = path("a1/b2") / path("c");
    auto p4 = path("a1/b2") / "c";
    auto p5 = path("a1") / "b2/c";
    auto p6 = path() / path("a1") / path("b2/c");

    CHECK_DIRECT(!p1.empty());
    CHECK_DIRECT(p1.is_valid());
    CHECK_DIRECT(!p1.is_absolute());
    CHECK_DIRECT(!p1.is_sub_root());
    CHECK_EQUALS_DIRECT(p1.size(), 3);
    CHECK_DIRECT(p1.root_name() == "a1");
    CHECK_DIRECT(p1.base_name() == "c");
    CHECK_DIRECT(p1.name(0) == "a1");
    CHECK_DIRECT(p1.name(1) == "b2");
    CHECK_DIRECT(p1.name(2) == "c");

    CHECK_DIRECT(!p2.empty());
    CHECK_DIRECT(p2.is_valid());
    CHECK_DIRECT(!p2.is_absolute());
    CHECK_DIRECT(!p2.is_sub_root());
    CHECK_EQUALS_DIRECT(p2.size(), 3);
    CHECK_DIRECT(p2.root_name() == "a1");
    CHECK_DIRECT(p2.base_name() == "c");
    CHECK_DIRECT(p2.name(0) == "a1");
    CHECK_DIRECT(p2.name(1) == "b2");
    CHECK_DIRECT(p2.name(2) == "c");

    CHECK_DIRECT(!p3.empty());
    CHECK_DIRECT(p3.is_valid());
    CHECK_DIRECT(!p3.is_absolute());
    CHECK_DIRECT(!p3.is_sub_root());
    CHECK_EQUALS_DIRECT(p3.size(), 3);
    CHECK_DIRECT(p3.root_name() == "a1");
    CHECK_DIRECT(p3.base_name() == "c");
    CHECK_DIRECT(p3.name(0) == "a1");
    CHECK_DIRECT(p3.name(1) == "b2");
    CHECK_DIRECT(p3.name(2) == "c");

    CHECK_DIRECT(!p4.empty());
    CHECK_DIRECT(p4.is_valid());
    CHECK_DIRECT(!p4.is_absolute());
    CHECK_DIRECT(!p4.is_sub_root());
    CHECK_EQUALS_DIRECT(p4.size(), 3);
    CHECK_DIRECT(p4.root_name() == "a1");
    CHECK_DIRECT(p4.base_name() == "c");
    CHECK_DIRECT(p4.name(0) == "a1");
    CHECK_DIRECT(p4.name(1) == "b2");
    CHECK_DIRECT(p4.name(2) == "c");

    CHECK_DIRECT(!p5.empty());
    CHECK_DIRECT(p5.is_valid());
    CHECK_DIRECT(!p5.is_absolute());
    CHECK_DIRECT(!p5.is_sub_root());
    CHECK_EQUALS_DIRECT(p5.size(), 3);
    CHECK_DIRECT(p5.root_name() == "a1");
    CHECK_DIRECT(p5.base_name() == "c");
    CHECK_DIRECT(p5.name(0) == "a1");
    CHECK_DIRECT(p5.name(1) == "b2");
    CHECK_DIRECT(p5.name(2) == "c");

    CHECK_DIRECT(!p6.empty());
    CHECK_DIRECT(p6.is_valid());
    CHECK_DIRECT(!p6.is_absolute());
    CHECK_DIRECT(!p6.is_sub_root());
    CHECK_EQUALS_DIRECT(p6.size(), 3);
    CHECK_DIRECT(p6.root_name() == "a1");
    CHECK_DIRECT(p6.base_name() == "c");
    CHECK_DIRECT(p6.name(0) == "a1");
    CHECK_DIRECT(p6.name(1) == "b2");
    CHECK_DIRECT(p6.name(2) == "c");
}

void test_sub_path(){
    path p1("/a/b/c/d/e/");
    path p2("a/b/c/d/e/");

    CHECK_DIRECT(p1.sub_path(0) == p1);
    CHECK_DIRECT(p1.sub_path(1) == path("a/b/c/d/e"));
    CHECK_DIRECT(p1.sub_path(2) == path("b/c/d/e"));
    CHECK_DIRECT(p1.sub_path(3) == path("c/d/e"));
    CHECK_DIRECT(p1.sub_path(4) == path("d/e"));
    CHECK_DIRECT(p1.sub_path(5) == path("e"));

    CHECK_DIRECT(p2.sub_path(0) == p2);
    CHECK_DIRECT(p2.sub_path(1) == path("b/c/d/e"));
    CHECK_DIRECT(p2.sub_path(2) == path("c/d/e"));
    CHECK_DIRECT(p2.sub_path(3) == path("d/e"));
    CHECK_DIRECT(p2.sub_path(4) == path("e"));
}

void test_branch_path(){
    path p1("/a/b/c/d/e/");
    path p2("a/b/c/d/e/");

    CHECK_DIRECT(p1.branch_path() == "/a/b/c/d/");
    CHECK_DIRECT(p1.branch_path() == "/a/b/c/d");

    CHECK_DIRECT(p2.branch_path() == "a/b/c/d/");
    CHECK_DIRECT(p2.branch_path() == "a/b/c/d");
}

void test_double_slash(){
    path p1("//");
    path p2("//a1//");
    path p3("//a1//b2//c");
    path p4("a1//b2/c//");

    CHECK_DIRECT(!p1.empty());
    CHECK_DIRECT(p1.is_valid());
    CHECK_DIRECT(p1.is_absolute());
    CHECK_DIRECT(p1.is_root());
    CHECK_DIRECT(!p1.is_sub_root());
    CHECK_EQUALS_DIRECT(p1.size(), 1);
    CHECK_DIRECT(p1.root_name() == "/");
    CHECK_DIRECT(p1.base_name() == "/");
    CHECK_DIRECT(p1.name(0) == "/");
    CHECK_DIRECT(p1 == "/");
    CHECK_DIRECT(p1 == "///");
    CHECK_DIRECT(p1.string() == "/");

    CHECK_DIRECT(!p2.empty());
    CHECK_DIRECT(p2.is_valid());
    CHECK_DIRECT(p2.is_absolute());
    CHECK_DIRECT(p2.is_sub_root());
    CHECK_EQUALS_DIRECT(p2.size(), 2);
    CHECK_DIRECT(p2.root_name() == "/");
    CHECK_DIRECT(p2.base_name() == "a1");
    CHECK_DIRECT(p2.sub_root_name() == "a1");
    CHECK_DIRECT(p2.name(0) == "/");
    CHECK_DIRECT(p2.name(1) == "a1");
    CHECK_DIRECT(p2 == "/a1/");
    CHECK_DIRECT(p2 == "///a1/////");
    CHECK_DIRECT(p2.string() == "/a1/");

    CHECK_DIRECT(!p3.empty());
    CHECK_DIRECT(p3.is_valid());
    CHECK_DIRECT(p3.is_absolute());
    CHECK_DIRECT(!p3.is_sub_root());
    CHECK_EQUALS_DIRECT(p3.size(), 4);
    CHECK_DIRECT(p3.root_name() == "/");
    CHECK_DIRECT(p3.base_name() == "c");
    CHECK_DIRECT(p3.sub_root_name() == "a1");
    CHECK_DIRECT(p3.name(0) == "/");
    CHECK_DIRECT(p3.name(1) == "a1");
    CHECK_DIRECT(p3.name(2) == "b2");
    CHECK_DIRECT(p3.name(3) == "c");
    CHECK_DIRECT(p3 == "/a1/b2/c");
    CHECK_DIRECT(p3 == "/a1//////b2/c//");
    CHECK_DIRECT(p3.string() == "/a1/b2/c/");

    CHECK_DIRECT(!p4.empty());
    CHECK_DIRECT(p4.is_valid());
    CHECK_DIRECT(!p4.is_absolute());
    CHECK_DIRECT(!p4.is_sub_root());
    CHECK_EQUALS_DIRECT(p4.size(), 3);
    CHECK_DIRECT(p4.root_name() == "a1");
    CHECK_DIRECT(p4.base_name() == "c");
    CHECK_DIRECT(p4.name(0) == "a1");
    CHECK_DIRECT(p4.name(1) == "b2");
    CHECK_DIRECT(p4.name(2) == "c");
    CHECK_DIRECT(p4 == "a1/b2/c");
    CHECK_DIRECT(p4 != "////a1/b2/c/////");
    CHECK_DIRECT(p4.string() == "a1/b2/c/");
}

} //end of anonymous namespace

void path_tests(){
    test_empty();
    test_root();
    test_path_0();
    test_path_1();
    test_path_2();
    test_path_3();
    test_invalidate();
    test_not_equals();
    test_concat_0();
    test_concat_1();
    test_sub_path();
    test_branch_path();
    test_double_slash();
}
