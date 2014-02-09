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
    std::string current_input;

    while(true){
        print("thor> ");

        auto c = read_input(input_buffer, 63);

        if(input_buffer[c-1] == '\n'){
            input_buffer[c-1] = '\0';

            current_input = input_buffer;

            if(current_input == "exit"){
                exit(0);
            } else if(current_input == "long"){
                //TODO Remove this function when exec system is complete
                exec_and_wait("long");
            } else if(current_input == "sleep"){
                //TODO Once better infrastrucure, parse command line and sleep the
                //correct number of milliseconds
                sleep_ms(5000);
            } else {
                print_line("Unknown command");
            }

            current_input.clear();
        } else {
            input_buffer[c] = '\0';

            current_input += input_buffer;
        }
    }
}