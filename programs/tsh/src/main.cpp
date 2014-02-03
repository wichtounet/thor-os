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
        print("thor> ");

        auto c = read_input(input_buffer, 63);

        if(input_buffer[c-1] == '\n'){
            if(c == 1){
                print_line();
                continue;
            }

            input_buffer[c] = '\0';

            if(str_equals(input_buffer, "exit")){
                exit(0);
            } else if(str_equals(input_buffer, "sleep")){
                //TODO Once better infrastrucure, parse command line and sleep the
                //correct number of milliseconds
                sleep_ms(5000);
            } else {
                print_line("Unknown command");
            }
        } else {
            //TODO Once in good shape, just append to current input buffer
        }

    }
}