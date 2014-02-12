//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ERRORS_H
#define ERRORS_H

namespace std {

constexpr const size_t ERROR_NOT_EXISTS = 1;
constexpr const size_t ERROR_NOT_EXECUTABLE = 2;
constexpr const size_t ERROR_FAILED_EXECUTION = 3;

inline const char* error_message(size_t error){
    switch(error){
        case ERROR_NOT_EXISTS:
            return "The file does not exist";
        case ERROR_NOT_EXECUTABLE:
            return "The file is not an executable";
        case ERROR_FAILED_EXECUTION:
            return "Execution failed";
        default:
            return "Unknonwn error";
    }
}

}

#endif
