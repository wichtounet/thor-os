//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <string.hpp>
#include <algorithms.hpp>

#include <tlib/print.hpp>
#include <tlib/file.hpp>
#include <tlib/system.hpp>
#include <tlib/errors.hpp>

namespace {

size_t columns = 0;
size_t rows  = 0;

void get_console_information(){
    columns = tlib::get_columns();
    rows = tlib::get_rows();
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
    tlib::exit(0);
}

void echo_command(const std::vector<std::string>& params){
    for(uint64_t i = 1; i < params.size(); ++i){
        tlib::print(params[i]);
        tlib::print(' ');
    }
    tlib::print_line();
}

void sleep_command(const std::vector<std::string>& params){
    if(params.size() == 1){
        tlib::print_line("sleep: missing operand");
    } else {
        size_t time = std::parse(params[1]);
        tlib::sleep_ms(time * 1000);
    }
}

void clear_command(const std::vector<std::string>&){
    tlib::clear();
}

void cd_command(const std::vector<std::string>& params){
    if(params.size() == 1){
        tlib::print_line("Usage: cd file_path");
        return;
    }

    auto& path = params[1];

    auto fd = tlib::open(path.c_str());

    if(fd.valid()){
        auto info = tlib::stat(*fd);

        if(info.valid()){
            if(!(info->flags & tlib::STAT_FLAG_DIRECTORY)){
                tlib::print_line("cat: error: Is not a directory");
            } else {
                if(path[0] == '/'){
                    tlib::set_current_working_directory(path);
                } else {
                    auto cwd = tlib::current_working_directory();

                    tlib::set_current_working_directory(cwd + "/" + path);
                }
            }
        } else {
            tlib::printf("cd: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("cd: error: %s\n", std::error_message(fd.error()));
    }
}

void pwd_command(const std::vector<std::string>&){
    auto cwd = tlib::current_working_directory();
    tlib::print_line(cwd);
}

} //end of anonymous namespace

int main(){
    get_console_information();

    char input_buffer[256];
    std::string current_input;

    tlib::print("thor> ");

    while(true){
        auto c = tlib::read_input(input_buffer, 255 );

        if(input_buffer[c-1] == '\n'){
            if(c > 1){
                input_buffer[c-1] = '\0';

                current_input += input_buffer;
            }

            if(current_input.size() > 0){
                auto params = std::split(current_input);

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

                    auto result = tlib::exec_and_wait(executable.c_str(), args);

                    if(!result.valid()){
                        tlib::print("error: ");
                        tlib::print_line(std::error_message(result.error()));
                        tlib::print("command: \"");
                        tlib::print(current_input);
                        tlib::print_line("\"");
                    }
                }
            }

            current_input.clear();

            tlib::print("thor> ");
        } else {
            input_buffer[c] = '\0';

            current_input += input_buffer;
        }
    }

    __builtin_unreachable();
}
