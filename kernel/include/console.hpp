#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.hpp"

void set_column(long column);
long get_column();

void set_line(long line);
long get_line();

void wipeout();
void k_print(char key);
void k_print(const char* string);
void k_print(uint64_t number);
void k_print_line();
void k_print_line(const char* string);
void k_printf(const char* fmt, ...);

#endif
