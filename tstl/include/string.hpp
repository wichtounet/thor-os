//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRING_H
#define STRING_H

#include <types.hpp>
#include <algorithms.hpp>
#include <vector.hpp>

namespace std {

inline uint64_t str_len(const char* a){
    uint64_t length = 0;
    while(*a++){
        ++length;
    }
    return length;
}

template<typename CharT>
struct basic_string {
public:
    typedef CharT*             iterator;
    typedef const CharT*       const_iterator;

    static constexpr const size_t npos = -1;

private:
    size_t _size;
    size_t _capacity;
    CharT* _data;

public:
    //Constructors

    basic_string() : _size(0), _capacity(1), _data(new CharT[1]) {
        _data[0] = '\0';
    }

    basic_string(const CharT* s) : _size(str_len(s)), _capacity(_size + 1), _data(new CharT[_capacity]) {
        std::copy_n(_data, s, _capacity);
    }

    explicit basic_string(size_t __capacity) : _size(0), _capacity(__capacity), _data(new CharT[_capacity]) {
        _data[0] = '\0';
    }

    //Copy constructors

    basic_string(const basic_string& rhs) : _size(rhs._size), _capacity(rhs._capacity), _data(new CharT[_capacity]) {
        std::copy_n(_data, rhs._data, _size + 1);
    }

    basic_string& operator=(const basic_string& rhs){
        if(this != &rhs){
            if(_capacity < rhs._capacity || !_data){
                if(_data){
                    delete[] _data;
                }

                _capacity = rhs._capacity;
                _data = new CharT[_capacity];
            }

            _size = rhs._size;
            std::copy_n(_data, rhs._data, _size + 1);
        }

        return *this;
    }

    //Move constructors

    basic_string(basic_string&& rhs) : _size(rhs._size), _capacity(rhs._capacity), _data(rhs._data) {
        rhs._size = 0;
        rhs._capacity = 0;
        rhs._data = nullptr;
    }

    basic_string& operator=(basic_string&& rhs){
        if(_data){
            delete[] _data;
        }

        _size = rhs._size;
        _capacity = rhs._capacity;
        _data = rhs._data;

        rhs._size = 0;
        rhs._capacity = 0;
        rhs._data = nullptr;

        return *this;
    }

    //Destructors

    ~basic_string(){
        if(_data){
            delete[] _data;
        }
    }

    //Modifiers

    void clear(){
        _size = 0;
        _data[0] = '\0';
    }

    void pop_back(){
        _data[--_size] = '\0';
    }

    basic_string operator+(CharT c) const {
        basic_string copy(*this);

        copy += c;

        return move(copy);
    }

    basic_string& operator+=(CharT c){
        if(!_data || _capacity <= _size + 1){
            _capacity = _capacity ? _capacity * 2 : 1;
            auto new_data = new CharT[_capacity];

            if(_data){
                std::copy_n(new_data, _data, _size);
                delete[] _data;
            }

            _data = new_data;
        }

        _data[_size] = c;
        _data[++_size] = '\0';

        return *this;
    }

    basic_string& operator+=(const char* rhs){
        auto len = str_len(rhs);
        if(!_data || _capacity <= _size + len){
            _capacity = _capacity ? _capacity * 2 : 1;

            if(_capacity < _size + len){
                _capacity = _size + len + 1;
            }

            auto new_data = new CharT[_capacity];

            if(_data){
                std::copy_n(new_data, _data, _size);
                delete[] _data;
            }

            _data = new_data;
        }

        std::copy_n(_data + _size, rhs, len);

        _size += len;

        _data[_size] = '\0';

        return *this;
    }

    basic_string& operator+=(const basic_string& rhs){
        if(!_data || _capacity <= _size + rhs.size()){
            _capacity = _capacity ? _capacity * 2 : 1;

            if(_capacity < _size + rhs.size()){
                _capacity = _size + rhs.size() + 1;
            }

            auto new_data = new CharT[_capacity];

            if(_data){
                std::copy_n(new_data, _data, _size);
                delete[] _data;
            }

            _data = new_data;
        }

        std::copy_n(_data + _size, rhs.c_str(), rhs.size());

        _size += rhs.size();

        _data[_size] = '\0';

        return *this;
    }

    //Accessors

    size_t size() const {
        return _size;
    }

    size_t capacity() const {
        return _capacity;
    }

    bool empty() const {
        return !_size;
    }

    CharT* c_str(){
        return _data;
    }

    const CharT* c_str() const {
        return _data;
    }

    CharT& operator[](size_t i){
        return _data[i];
    }

    const CharT& operator[](size_t i) const {
        return _data[i];
    }

    size_t find(char c) const {
        for(size_t i = 0; i < size(); ++i){
            if(_data[i] == c){
                return i;
            }
        }

        return npos;
    }

    //Operators

    bool operator==(const CharT* s) const {
        if(size() != str_len(s)){
            return false;
        }

        for(size_t i = 0; i < size(); ++i){
            if(_data[i] != s[i]){
                return false;
            }
        }

        return true;
    }

    bool operator==(const basic_string& rhs) const {
        if(size() != rhs.size()){
            return false;
        }

        for(size_t i = 0; i < size(); ++i){
            if(_data[i] != rhs._data[i]){
                return false;
            }
        }

        return true;
    }

    //Iterators

    iterator begin(){
        return iterator(&_data[0]);
    }

    iterator end(){
        return iterator(&_data[_size]);
    }

    const_iterator begin() const {
        return const_iterator(&_data[0]);
    }

    const_iterator end() const {
        return const_iterator(&_data[_size]);
    }
};

template<typename C>
basic_string<C> operator+(const basic_string<C>& lhs, const basic_string<C>& rhs){
    basic_string<C> result;
    result += lhs;
    result += rhs;
    return std::move(result);
}

template<typename C>
basic_string<C> operator+(const C* lhs, const basic_string<C>& rhs){
    basic_string<C> result;
    result += lhs;
    result += rhs;
    return std::move(result);
}

template<typename C>
basic_string<C> operator+(const basic_string<C>& lhs, const C* rhs){
    basic_string<C> result;
    result += lhs;
    result += rhs;
    return std::move(result);
}

typedef basic_string<char> string;

inline uint64_t parse(const char* it, const char* end){
    int i = end - it - 1;

    uint64_t factor = 1;
    uint64_t acc = 0;

    for(; i >= 0; --i){
        acc += (it[i] - '0') * factor;
        factor *= 10;
    }

    return acc;
}

inline uint64_t parse(const char* str){
    int i = 0;

    const char* it = str;
    while(*++it){
        ++i;
    }

    uint64_t factor = 1;
    uint64_t acc = 0;

    for(; i >= 0; --i){
        acc += (str[i] - '0') * factor;
        factor *= 10;
    }

    return acc;
}

inline uint64_t parse(const string& str){
    return parse(str.begin(), str.end());
}

template<typename N>
size_t digits(N number){
    if(number < 10){
        return 1;
    }

    size_t i = 0;

    while(number != 0){
        number /= 10;
        ++i;
    }

    return i;
}

template<typename Char>
std::vector<std::basic_string<Char>> split(const std::basic_string<Char>& s, char sep = ' '){
    std::vector<std::basic_string<Char>> parts;

    std::basic_string<Char> current(s.size());

    for(char c : s){
        if(c == sep && !current.empty()){
            parts.push_back(current);
            current.clear();
        } else if(c == sep){
            continue;
        } else {
            current += c;
        }
    }

    if(!current.empty()){
        parts.push_back(current);
    }

    return std::move(parts);
}

template<typename T>
std::string to_string(const T& value);

template<>
inline std::string to_string<uint64_t>(const uint64_t& value){
    if(value == 0){
        return "0";
    }

    std::string s;

    char buffer[20];
    int i = 0;
    auto rem = value;

    while(rem != 0){
        buffer[i++] = '0' + rem  % 10;
        rem /= 10;
    }

    --i;

    for(; i >= 0; --i){
        s += buffer[i];
    }

    return std::move(s);
}

template<>
inline std::string to_string<int64_t>(const int64_t& value){
    if(value < 0){
        std::string s("-");
        s += to_string(static_cast<uint64_t>(value));
        return std::move(s);
    } else {
        return to_string(static_cast<uint64_t>(value));
    }
}

template<>
inline std::string to_string<uint8_t>(const uint8_t& value){
    return to_string(static_cast<uint64_t>(value));
}

template<>
inline std::string to_string<uint16_t>(const uint16_t& value){
    return to_string(static_cast<uint64_t>(value));
}

template<>
inline std::string to_string<uint32_t>(const uint32_t& value){
    return to_string(static_cast<uint64_t>(value));
}

template<>
inline std::string to_string<int8_t>(const int8_t& value){
    return to_string(static_cast<int64_t>(value));
}

template<>
inline std::string to_string<int16_t>(const int16_t& value){
    return to_string(static_cast<int64_t>(value));
}

template<>
inline std::string to_string<int32_t>(const int32_t& value){
    return to_string(static_cast<int64_t>(value));
}

} //end of namespace std

#endif
