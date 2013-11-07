//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

; Some utility macros

; Define a string and a variable containing its length
%macro STRING 2
    %1 db %2, 0
    %1_length equ $ - %1 - 1
%endmacro
