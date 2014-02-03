//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USERLIB_STRING_H
#define USERLIB_STRING_H

int str_compare(const char *s1, const char *s2){
    while (*s1 != '\0') {
        if (*s2 == '\0'){
            return  1;
        }

        if (*s2 > *s1){
            return -1;
        }

        if (*s1 > *s2){
            return  1;
        }

        s1++;
        s2++;
    }

    if (*s2 != '\0'){
        return -1;
    }

    return 0;
}

bool str_equals(const char* s1, const char* s2){
    return str_compare(s1, s2) == 0;
}

#endif
