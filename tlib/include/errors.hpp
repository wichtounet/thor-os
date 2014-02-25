//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ERRORS_H
#define ERRORS_H

#include <types.hpp>

//TODO Rename this namespace
namespace std {

//TODO Use an enum
constexpr const size_t ERROR_NOT_EXISTS = 1;
constexpr const size_t ERROR_NOT_EXECUTABLE = 2;
constexpr const size_t ERROR_FAILED_EXECUTION = 3;
constexpr const size_t ERROR_NOTHING_MOUNTED = 4;
constexpr const size_t ERROR_INVALID_FILE_PATH = 5;
constexpr const size_t ERROR_DIRECTORY = 6;
constexpr const size_t ERROR_INVALID_FILE_DESCRIPTOR= 7;
constexpr const size_t ERROR_FAILED= 8;

inline const char* error_message(size_t error){
    switch(error){
        case ERROR_NOT_EXISTS:
            return "The file does not exist";
        case ERROR_NOT_EXECUTABLE:
            return "The file is not an executable";
        case ERROR_FAILED_EXECUTION:
            return "Execution failed";
        case ERROR_NOTHING_MOUNTED:
            return "Nothing is mounted";
        case ERROR_INVALID_FILE_PATH:
            return "The file path is not valid";
        case ERROR_DIRECTORY:
            return "The file is a directory";
        case ERROR_INVALID_FILE_DESCRIPTOR:
            return "Invalid file descriptor";
        case ERROR_FAILED:
            return "Failed";
        default:
            return "Unknonwn error";
    }
}

}

#endif
