#include <cstddef>

#include "console.hpp"
#include "types.hpp"

namespace {

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

}

void set_column(long column){
    current_column = column;
}

long get_column(){
    return current_column;
}

void k_print_line(){
    current_column = 0;
    ++current_line;
}

void k_print_line(const char* string){
    k_print(string);

    current_column = 0;
    ++current_line;
}

void k_print(std::size_t number){
    char buffer[20];
    int i = 0;

    while(number != 0){
        buffer[i++] = '0' + number % 10;
        number /= 10;
    }

    --i;

    for(; i >= 0; --i){
        k_print(buffer[i]);
    }
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

void wipeout(){
    current_line = 0;
    current_column = 0;

    for(int line = 0; line < 25; ++line){
        for(std::size_t column = 0; column < 80; ++column){
            k_print(' ');
        }
    }

    current_line = 0;
    current_column = 0;
}
