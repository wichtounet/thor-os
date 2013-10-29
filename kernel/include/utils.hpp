#ifndef UTILS_H
#define UTILS_H

#include <cstddef>

std::size_t parse(const char* str);

bool str_contains(const char* a, char c);
void str_copy(const char* a, char* b);
const char* str_until(char* a, char c);
const char* str_from(char* a, char c);

#endif
