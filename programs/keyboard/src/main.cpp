//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/print.hpp>

const char* source = "Hello world";

int main(){
    char buffer[16];

    tlib::printf("Read 1 string (max 15)\n");

    auto c = tlib::read_input(buffer, 15);
    buffer[c] = '\0';
    tlib::print(buffer);

    tlib::printf("Read 1 string (max 15) with timeout 5\n");
    c = tlib::read_input(buffer, 15, 5000);

    if(c){
        buffer[c] = '\0';
        tlib::print(buffer);
    } else {
        tlib::printf("Timeout reached\n");
    }

    return 0;
}
