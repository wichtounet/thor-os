//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
