#include <cstddef>

#include "types.hpp"
#include "addresses.hpp"
#include "keyboard.hpp"

void k_print(char key);
void k_print(const char* string);
void k_print_line(const char* string);

uint8_t in_byte(uint16_t _port){
    uint8_t rv;
    __asm__ __volatile__ ("in %0, %1" : "=a" (rv) : "dN" (_port));
    return rv;
}

void out_byte (uint16_t _port, uint8_t _data){
    __asm__ __volatile__ ("out %0, %1" : : "dN" (_port), "a" (_data));
}

template<uint8_t IRQ>
void register_irq_handler(void (*handler)()){
    asm ("mov r8, %0; mov r9, %1; call %2"
        :
        : "I" (IRQ), "r" (handler), "i" (asm_register_irq_handler)
        : "r8", "r9"
        );
}

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

           current_input[current_input_length++] = qwertz_key;
           k_print(qwertz_key);
        }
    }
}

long current_line = 0;
long current_column = 0;

enum vga_color {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GREY = 7,
    DARK_GREY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    LIGHT_BROWN = 14,
    WHITE = 15,
};

uint8_t make_color(vga_color fg, vga_color bg){
    return fg | bg << 4;
}

uint16_t make_vga_entry(char c, uint8_t color){
    uint16_t c16 = c;
    uint16_t color16 = color;
    return c16 | color16 << 8;
}

void k_print_line(const char* string){
    k_print(string);

    current_column = 0;
    ++current_line;
}

void k_print(char key){
    uint16_t* vga_buffer = (uint16_t*) 0x0B8000;

    vga_buffer[current_line * 80 + current_column] = make_vga_entry(key, make_color(WHITE, BLACK));

    ++current_column;

    return;
}

void k_print(const char* string){
    for(int i = 0; string[i] != 0; ++i){
        k_print(string[i]);
    }

    return;
}
