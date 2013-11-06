#ifndef UTILS_H
#define UTILS_H

#include "types.hpp"

uint64_t parse(const char* str);
uint64_t parse(const char* str, const char* end);

bool str_equals(const char* a, const char* b);
bool str_contains(const char* a, char c);
void str_copy(const char* a, char* b);
const char* str_until(char* a, char c);
const char* str_from(char* a, char c);

#endif
