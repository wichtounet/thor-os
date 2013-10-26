void k_print_line(const char* string);
void k_print(const char* string);

extern "C"
void  __attribute__ ((section ("main_section"))) kernel_main(){
    k_print_line("hello, world!");

    return;
}

typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));

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

long current_line = 0;
long current_column = 0;

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

void k_print(const char* string){
    uint16_t* vga_buffer = (uint16_t*) 0x0B8000;

    for(int i = 0; string[i] != 0; ++i){
        vga_buffer[current_line * 80 + current_column] = make_vga_entry(string[i], make_color(WHITE, BLACK));

        ++current_column;
    }

    return;
}
