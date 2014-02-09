//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <system.hpp>
#include <string.hpp>
#include <algorithms.hpp>

int main(){
    char input_buffer[64];
    std::string current_input;

    while(true){
        print("thor> ");

        auto c = read_input(input_buffer, 63);

        if(input_buffer[c-1] == '\n'){
            input_buffer[c-1] = '\0';

            current_input += input_buffer;

            auto params = std::split(current_input);;

            if(params[0] == "exit"){
                exit(0);
            } else if(params[0] == "sleep"){
                if(params.size() == 1){
                    print_line("sleep: missing operand");
                } else {
                    size_t time = std::parse(params[1]);
                    sleep_ms(time * 1000);
                }
            } else {
                auto result = exec_and_wait(params[0].c_str());

                if(!result.valid()){
                    print("error: ");

                    auto err = result.error();
                    if(err == 1){
                        print_line("The file does not exist");
                    } else if(err == 2){
                        print_line("The file is not an executable");
                    } else if(err == 3){
                        print_line("Failed to execute the file");
                    }
                }
            }

            current_input.clear();
        } else {
            input_buffer[c] = '\0';

            current_input += input_buffer;
        }
    }
}