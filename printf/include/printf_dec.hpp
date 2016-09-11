//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

//Is made to be included as is in a header file
//
//Implement is provided in printf_def.hpp which must be included in
//source file

std::string sprintf(const std::string& format, ...);
std::string vsprintf(const std::string& format, va_list va);

void printf(const std::string& format, ...);
void vprintf(const std::string& format, va_list va);

void sprintf_raw(char* buffer, size_t n, const char* format, ...);
void vsprintf_raw(char* buffer, size_t n, const char* format, va_list va);

void printf_raw(const char* format, ...);
void vprintf_raw(const char* format, va_list va);

//Definition of these functions must be provided
void __printf(const std::string& formatted);
void __printf_raw(const char* formatted);
