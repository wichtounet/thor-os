//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
