//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <system.hpp>
#include <datetime.hpp>

constexpr const size_t PAGES = 256;

//TODO Need a ms timestamp at least

int main(){
    char* buffer_one = new char[PAGES * 4096];
    char* buffer_two = new char[PAGES * 4096];

    auto date = local_date();
    auto start = date.hour * 3600 + date.minutes * 60 + date.seconds;

    std::copy_n(buffer_one, buffer_two, PAGES * 4096);

    date = local_date();
    auto end = date.hour * 3600 + date.minutes * 60 + date.seconds;

    printf("Copy took %u \n", end - start);

    exit(0);
}
