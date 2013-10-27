#include <cstddef>

#include "types.hpp"
#include "keyboard.hpp"
#include "kernel_utils.hpp"
#include "console.hpp"

void keyboard_handler();

extern "C" {
void  __attribute__ ((section ("main_section"))) kernel_main(){
    k_print("thor> ");

    register_irq_handler<1>(keyboard_handler);

    return;
}
}

std::size_t current_input_length = 0;
char current_input[50];

void keyboard_handler(){
    uint8_t key = in_byte(0x60);

    if(key & 0x80){
        //TODO Handle shift
    } else {
        if(key == 0x1C){
            //TODO Enter
        } else if(key == 0x0E){
            //TODO Backspace
        } else {
           auto qwertz_key = key_to_ascii(key);

           if(qwertz_key > 0){
               current_input[current_input_length++] = qwertz_key;
               k_print(qwertz_key);
           }
        }
    }
}
