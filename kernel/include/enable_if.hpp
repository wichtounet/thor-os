#ifndef ENABLE_IF_H
#define ENABLE_IF_H

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

template<bool B, class T = void>
struct disable_if {};

template<class T>
struct disable_if<false, T> { typedef T type; };

#endif
