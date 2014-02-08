//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint64_t __attribute__ ((__mode__ (__DI__)));

typedef int int8_t __attribute__((__mode__(__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));

typedef uint64_t uintptr_t;
typedef uint64_t size_t;

static_assert(sizeof(uint8_t) == 1, "uint8_t must be 1 byte long");
static_assert(sizeof(uint16_t) == 2, "uint16_t must be 2 bytes long");
static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes long");
static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes long");

static_assert(sizeof(int8_t) == 1, "int8_t must be 1 byte long");
static_assert(sizeof(int16_t) == 2, "int16_t must be 2 bytes long");
static_assert(sizeof(int32_t) == 4, "int32_t must be 4 bytes long");
static_assert(sizeof(int64_t) == 8, "int64_t must be 8 bytes long");

static_assert(sizeof(size_t) == 8, "size_t must be 8 bytes long");

#ifndef CODE_32
#ifndef CODE_16
static_assert(sizeof(uintptr_t) == sizeof(uint64_t*), "uintptr_t must have the same size as a pointer");
#endif
#endif

#endif
