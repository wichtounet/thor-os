#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint64_t __attribute__ ((__mode__ (__DI__)));

typedef uint64_t uintptr_t;

static_assert(sizeof(uint8_t) == 1, "uint32_t must be 1 byte long");
static_assert(sizeof(uint16_t) == 2, "uint32_t must be 2 bytes long");
static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes long");
static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes long");
static_assert(sizeof(uintptr_t) == sizeof(uint64_t*), "uintptr_t must have the same size as a pointer");

#endif
