//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint64_t __attribute__ ((__mode__ (__DI__)));

const char* source = "Hello world";

uint8_t make_color(uint8_t fg, uint8_t bg){
    return fg | bg << 4;
}

uint16_t make_vga_entry(char c, uint8_t color){
    uint16_t c16 = c;
    uint16_t color16 = color;
    return c16 | color16 << 8;
}

int main(){
    uint16_t* vga_buffer = reinterpret_cast<uint16_t*>(0x0B8000);

    auto s = source;
    uint64_t i = 0;

    while(*s){
        vga_buffer[10 * 80 + 20 + i * 2] = make_vga_entry(*s, make_color(15, 0));
        ++s;
        ++i;
    }

    return 0;
}