//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <tlib/print.hpp>
#include <tlib/system.hpp>

int main(int, char*[]){
    auto date = tlib::local_date();

    tlib::print(date.day);
    tlib::print('.');
    tlib::print(date.month);
    tlib::print('.');
    tlib::print(date.year);
    tlib::print(' ');

    tlib::print(date.hour);
    tlib::print(':');
    tlib::print(date.minutes);
    tlib::print_line();

    return 0;
}
