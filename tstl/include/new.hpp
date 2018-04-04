//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
