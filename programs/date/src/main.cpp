//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <system.hpp>
#include <print.hpp>

int main(int, char*[]){
    auto date = local_date();

    print(date.day);
    print('.');
    print(date.month);
    print('.');
    print(date.year);
    print(' ');

    print(date.hour);
    print(':');
    print(date.minutes);
    print_line();

    exit(0);
}