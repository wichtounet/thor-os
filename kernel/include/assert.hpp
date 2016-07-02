//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASSERT_HPP
#define ASSERT_HPP

void __thor_assert(bool condition);
void __thor_assert(bool condition, const char* message);
void __thor_unreachable(const char* message);

#ifdef NASSERT
inline void thor_assert(bool){}
inline void thor_assert(bool, const char*){}
#else
inline void thor_assert(bool condition){
    __thor_assert(condition);
}

inline void thor_assert(bool condition, const char* message){
    __thor_assert(condition, message);
}
#endif

inline void thor_unreachable(const char* message) __attribute__((noreturn));
inline void thor_unreachable(const char* message){
    __thor_unreachable(message);
    __builtin_unreachable();
}

#endif
