#ifndef TYPES_H
#define TYPES_H

#include <cstddef>

typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));

static_assert(sizeof(uint8_t) == 1, "The size must match");
static_assert(sizeof(uint16_t) == 2, "The size must match");
static_assert(sizeof(uint32_t) == 4, "The size must match");

#endif
