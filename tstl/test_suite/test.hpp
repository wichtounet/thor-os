//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>

#define CHECK(cond, message) check(cond, message, __PRETTY_FUNCTION__, __LINE__)
#define CHECK_EQUALS(a, b, message) check_equals(a, b, message, __PRETTY_FUNCTION__, __LINE__)

#define CHECK_DIRECT(cond) check(cond, __PRETTY_FUNCTION__, __LINE__)
#define CHECK_EQUALS_DIRECT(a, b) check_equals(a, b, __PRETTY_FUNCTION__, __LINE__)

void check(bool condition);
void check(bool condition, const char* message);
void check_equals(long value, long expected, const char* message);

void check(bool condition, const char* where, size_t line);
void check(bool condition, const char* message, const char* where, size_t line);
void check_equals(long value, long expected, const char* message, const char* where, size_t line);
void check_equals(long value, long expected, const char* where, size_t line);
