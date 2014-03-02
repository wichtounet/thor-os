//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <file.hpp>
#include <system.hpp>
#include <string.hpp>
#include <algorithms.hpp>
#include <errors.hpp>

namespace {

size_t columns = 0;
size_t rows  = 0;

void get_console_information(){
    columns = get_columns();
    rows = get_rows();
}

void exit_command(const std::vector<std::string>& params);
void echo_command(const std::vector<std::string>& params);
void sleep_command(const std::vector<std::string>& params);
void clear_command(const std::vector<std::string>& params);
void cd_command(const std::vector<std::string>& params);
void pwd_command(const std::vector<std::string>& params);

struct command_definition {
    const char* name;
    void (*function)(const std::vector<std::string>&);
};

command_definition commands[6] = {
    {"exit", exit_command},
    {"echo", echo_command},
    {"sleep", sleep_command},
    {"clear", clear_command},
    {"cd", cd_command},
    {"pwd", pwd_command},
};

void exit_command(const std::vector<std::string>&){
    exit(0);
}

void echo_command(const std::vector<std::string>& params){
    for(uint64_t i = 1; i < params.size(); ++i){
        print(params[i]);
        print(' ');
    }
    print_line();
}

void sleep_command(const std::vector<std::string>& params){
    if(params.size() == 1){
        print_line("sleep: missing operand");
    } else {
        size_t time = std::parse(params[1]);
        sleep_ms(time * 1000);
    }
}

void clear_command(const std::vector<std::string>&){
    clear();
}

void cd_command(const std::vector<std::string>& params){
    if(params.size() == 1){
        print_line("Usage: cd file_path");
        return;
    }

    auto& path = params[1];

    auto fd = open(path.c_str());

    if(fd.valid()){
        auto info = stat(*fd);

        if(info.valid()){
            if(!(info->flags & STAT_FLAG_DIRECTORY)){
                print_line("cat: error: Is not a directory");
            } else {
                auto cwd = current_working_directory();

                if(path[0] == '/'){
                    cwd = "/";
                }

                auto parts = std::split(path, '/');
                for(auto& part : parts){
                    cwd += part;
                    cwd += '/';
                }

                set_current_working_directory(cwd);
            }
        } else {
            printf("cd: error: %s\n", std::error_message(info.error()));
        }

        close(*fd);
    } else {
        printf("cd: error: %s\n", std::error_message(fd.error()));
    }
}

void pwd_command(const std::vector<std::string>&){
    auto cwd = current_working_directory();
    print_line(cwd);
}

} //end of anonymous namespace

int main(){
    get_console_information();

    char input_buffer[64];
    std::string current_input;

    while(true){
        print("thor> ");

        auto c = read_input(input_buffer, 63);

        if(input_buffer[c-1] == '\n'){
            if(c > 1){
                input_buffer[c-1] = '\0';

                current_input += input_buffer;
            }

            if(current_input.size() > 0){
                auto params = std::split(current_input);;

                bool found = false;
                for(auto& command : commands){
                    if(params[0] == command.name){
                        command.function(params);
                        found = true;
                        break;
                    }
                }

                if(!found){
                    std::vector<std::string> args;
                    if(params.size() > 1){
                        args.reserve(params.size() - 1);
                        for(size_t i = 1; i < params.size(); ++i){
                            args.push_back(params[i]);
                        }
                    }

                    auto executable = params[0];

                    if(executable[0] != '/'){
                        executable = "/bin/" + executable;
                    }

                    auto result = exec_and_wait(executable.c_str(), args);

                    if(!result.valid()){
                        print("error: ");
                        print_line(std::error_message(result.error()));
                        print("command: \"");
                        print(current_input);
                        print_line("\"");
                    }
                }
            }

            current_input.clear();
        } else {
            input_buffer[c] = '\0';

            current_input += input_buffer;
        }
    }

    __builtin_unreachable();
}