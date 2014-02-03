//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <system.hpp>
#include <string.hpp>

int main(){
    char input_buffer[64];

    while(true){
        for(int i = 0; i < 10; ++i){
            auto c = read_input(input_buffer, 63);
            input_buffer[c-1] = '\0';

            if(str_equals(input_buffer, "exit")){
                exit(0);
            } else {
                print_line("Unknown command");
            }
        }
    }
}