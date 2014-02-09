//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NEW_H
#define NEW_H

inline void* operator new( size_t , void* place){
    return place;
}

inline void* operator new[]( size_t , void* place){
    return place;
}

#endif
