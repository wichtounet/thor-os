//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint8_t __attribute__((__mode__(__QI__))); ///< An unsigned 8-bit number
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__))); ///< An unsigned 16-bit number
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__))); ///< An unsigned 32-bit number
typedef unsigned int uint64_t __attribute__ ((__mode__ (__DI__))); ///< An unsigned 64-bit number

typedef int int8_t __attribute__((__mode__(__QI__))); ///< A signed 8-bit number
typedef int int16_t __attribute__ ((__mode__ (__HI__))); ///< A signed 16-bit number
typedef int int32_t __attribute__ ((__mode__ (__SI__))); ///< A signed 32-bit number
typedef int int64_t __attribute__ ((__mode__ (__DI__))); ///< A signed 64-bit number

typedef uint64_t uintptr_t; ///< Type that can be used to store a pointer value
typedef uint64_t size_t; ///< Type that can be used to store the size of a collectiotn

typedef double sse_128 __attribute__((vector_size(16))); ///< SSE 128-bit value

static_assert(sizeof(uint8_t) == 1, "uint8_t must be 1 byte long");
static_assert(sizeof(uint16_t) == 2, "uint16_t must be 2 bytes long");
static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes long");
static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes long");

static_assert(sizeof(int8_t) == 1, "int8_t must be 1 byte long");
static_assert(sizeof(int16_t) == 2, "int16_t must be 2 bytes long");
static_assert(sizeof(int32_t) == 4, "int32_t must be 4 bytes long");
static_assert(sizeof(int64_t) == 8, "int64_t must be 8 bytes long");

static_assert(sizeof(size_t) == 8, "size_t must be 8 bytes long");

static_assert(sizeof(sse_128) == 16, "xmm registers are 16 bytes long");

#ifndef CODE_32
#ifndef CODE_16
static_assert(sizeof(uintptr_t) == sizeof(uint64_t*), "uintptr_t must have the same size as a pointer");
#endif
#endif

#endif
