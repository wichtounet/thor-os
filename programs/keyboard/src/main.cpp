//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <tlib/print.hpp>

const char* source = "Hello world";

int main(){
    char buffer[16];

    printf("Read 1 string (max 15)\n");

    auto c = read_input(buffer, 15);
    buffer[c] = '\0';
    print(buffer);

    printf("Read 1 string (max 15) with timeout 5\n");
    c = read_input(buffer, 15, 5000);

    if(c){
        buffer[c] = '\0';
        print(buffer);
    } else {
        printf("Timeout reached\n");
    }

    return 0;
}
