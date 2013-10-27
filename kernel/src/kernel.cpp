#include <cstddef>

#include "types.hpp"
#include "keyboard.hpp"
#include "kernel_utils.hpp"
#include "console.hpp"

void keyboard_handler();

extern "C" {
void  __attribute__ ((section ("main_section"))) kernel_main(){
    wipeout();

    k_print("thor> ");

    register_irq_handler<1>(keyboard_handler);

    return;
}
}

std::size_t current_input_length = 0;
char current_input[50];

void exec_command();

void keyboard_handler(){
    uint8_t key = in_byte(0x60);

    if(key & 0x80){
        //TODO Handle shift
    } else {
        if(key == 0x1C){
            current_input[current_input_length] = '\0';

            k_print_line();

            exec_command();

            current_input_length = 0;

            k_print("thor> ");
        } else if(key == 0x0E){
            set_column(get_column() - 1);
            k_print(' ');
            set_column(get_column() - 1);

            --current_input_length;
        } else {
           auto qwertz_key = key_to_ascii(key);

           if(qwertz_key > 0){
               current_input[current_input_length++] = qwertz_key;
               k_print(qwertz_key);
           }
        }
    }
}

bool str_equals(const char* a, const char* b){
    while(*a && *a == *b){
        ++a;
        ++b;
    }

    return *a == *b;
}

#define COMMANDS 2

struct command_definition {
    const char* name;
    void (*function)();
};

void reboot_command();
void help_command();

command_definition commands[COMMANDS] = {
    {"reboot", reboot_command},
    {"help", help_command}
};

void reboot_command(){
    interrupt<60>();
}

void help_command(){
    k_print_line("Available commands:");

    for(int i = 0; i < COMMANDS; ++i){
        k_print("   ");
        k_print_line(commands[i].name);
    }
}

void exec_command(){
    for(int i = 0; i < COMMANDS; ++i){
        if(str_equals(current_input, commands[i].name)){
            commands[i].function();

            return;
        }
    }

    k_print("The command \"");
    k_print(current_input);
    k_print_line("\" does not exist");
}
