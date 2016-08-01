//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRING_H
#define STRING_H

#include <types.hpp>
#include <algorithms.hpp>
#include <vector.hpp>
#include <unique_ptr.hpp>

namespace std {

inline uint64_t str_len(const char* a){
    uint64_t length = 0;
    while(*a++){
        ++length;
    }
    return length;
}

template<typename CharT>
struct base_long {
    size_t capacity;
    unique_ptr<CharT[]> data;

    base_long() = default;

    base_long(size_t capacity, CharT* array) : capacity(capacity), data(array) {}

    base_long(base_long&) = delete;
    base_long& operator=(base_long&) = delete;

    base_long(base_long&&) = default;
    base_long& operator=(base_long&&) = default;
};

static constexpr const size_t min_capacity = 16;
static constexpr const size_t words = min_capacity / sizeof(size_t);

//TODO Store size into base and use only one unsigned char for base_short

template<typename CharT>
struct base_short {
    CharT data[min_capacity];

    base_short() = default;

    base_short(base_short&) = delete;
    base_short& operator=(base_short&) = delete;

    base_short(base_short&&) = default;
    base_short& operator=(base_short&&) = default;
};

struct base_raw {
    size_t data[words];

    base_raw() = delete;

    base_raw(base_raw&) = delete;
    base_raw& operator=(base_raw&) = delete;

    base_raw(base_raw&&) = delete;
    base_raw& operator=(base_raw&&) = delete;
};

template<typename CharT>
union base_storage {
    base_short<CharT> small;
    base_long<CharT> big;
    base_raw raw;

    base_storage(){
        //Default construction: Nothing to do
    }

    ~base_storage() {}
};

static_assert(min_capacity == sizeof(base_short<char>), "base_short must be the correct SSO size");
static_assert(min_capacity == sizeof(base_long<char>), "base_long must be the correct SSO size");
static_assert(min_capacity == sizeof(base_raw), "base_raw must be the correct SSO size");

template<typename CharT>
struct basic_string {
public:
    typedef CharT*             iterator;
    typedef const CharT*       const_iterator;

    static constexpr const size_t npos = -1;

private:
    size_t _size;

    base_storage<CharT> storage;

    void set_long(bool small){
        if(small){
            _size |= (1UL << 63);
        } else {
            _size &= ~(1UL << 63);
        }
    }

    void set_small(bool small){
        set_long(!small);
    }

    void set_size(size_t size){
        if(is_long()){
            _size = size | (1UL << 63);
        } else {
            _size = size;
        }
    }

    bool is_long() const {
        return _size & (1UL << 63);
    }

    bool is_small() const {
        return !is_long();
    }

    void zero(){
        for(size_t i = 0; i < words; ++i){
            storage.raw.data[i] = 0;
        }
    }

public:
    //Constructors

    basic_string() : _size(0){
        set_small(true);

        (*this)[0] = '\0';
    }

    basic_string(const CharT* s) : _size(str_len(s)) {
        auto capacity = size() + 1;

        set_small(capacity <= 16);

        if(!is_small()){
            new (&storage.big) base_long<CharT>(capacity, new CharT[capacity]);
        }

        std::copy_n(begin(), s, capacity);
    }

    explicit basic_string(size_t __capacity) : _size(0) {
        set_small(__capacity <= 16);

        if(!is_small()){
            new (&storage.big) base_long<CharT>(__capacity, new CharT[__capacity]);
        }

        (*this)[0] = '\0';
    }

    //Copy

    basic_string(const basic_string& rhs) : _size(rhs._size) {
        if(!is_small()){
            new (&storage.big) base_long<CharT>(size() + 1, new CharT[size() + 1]);
        }

        std::copy_n(begin(), rhs.begin(), size() + 1);
    }

    basic_string& operator=(const basic_string& rhs){
        if(this != &rhs){
            set_size(rhs.size());

            if(capacity() < rhs.capacity()){
                auto capacity = rhs.capacity();

                if(is_small()){
                    new (&storage.big) base_long<CharT>(capacity, new CharT[capacity]);

                    set_small(false);
                } else {
                    storage.big.capacity = capacity;
                    storage.big.data.reset(new CharT[capacity]);
                }
            }

            std::copy_n(begin(), rhs.begin(), size() + 1);
        }

        return *this;
    }

    //Move

    basic_string(basic_string&& rhs) : _size(rhs._size) {
        if(is_small()){
            new (&storage.small) base_short<CharT>(std::move(rhs.storage.small));
        } else {
            new (&storage.big) base_long<CharT>(std::move(rhs.storage.big));
        }

        rhs._size = 0;
        rhs.zero();
    }

    basic_string& operator=(basic_string&& rhs){
        auto was_small = is_small();
        auto was_long = !was_small;

        auto small = rhs.is_small();
        auto lng = !small;

        set_size(rhs.size());

        if(was_small && small){
            storage.small = std::move(rhs.storage.small);
        } else if(was_long && lng){
            storage.big = std::move(rhs.storage.big);
        } else if(was_small && lng){
            new (&storage.big) base_long<CharT>(std::move(rhs.storage.big));

            set_small(false);
        } else if(was_long && small){
            ensure_capacity(rhs.size() + 1);

            std::copy_n(begin(), rhs.begin(), size() + 1);
        }

        rhs._size = 0;
        rhs.zero();

        return *this;
    }

    //Destructors

    ~basic_string(){
        if(is_long()){
            storage.big.~base_long();
        }
    }

    //Modifiers

    void adjust_size(size_t size){
        set_size(size);
    }

    void clear(){
        set_size(0);
        (*this)[0] = '\0';
    }

    void pop_back(){
        set_size(size() - 1);
        (*this)[size()] = '\0';
    }

    void reserve(size_t new_capacity){
        ensure_capacity(new_capacity);
    }

    basic_string operator+(CharT c) const {
        basic_string copy(*this);

        copy += c;

        return move(copy);
    }

    basic_string& operator+=(CharT c){
        ensure_capacity(size() + 2);

        (*this)[size()] = c;
        (*this)[size() + 1] = '\0';

        set_size(size() + 1);

        return *this;
    }

    void ensure_capacity(size_t new_capacity){
        if(new_capacity > 0 && (capacity() < new_capacity)){
            auto new_cap = capacity() * 2;

            if(new_cap < new_capacity){
                new_cap = new_capacity;
            }

            auto new_data = new CharT[new_cap];

            std::copy_n(new_data, begin(), size() + 1);

            if(is_small()){
                new (&storage.big) base_long<CharT>(new_cap, new_data);

                set_small(false);
            } else {
                storage.big.data.reset(new_data);
                storage.big.capacity = new_cap;
            }
        }
    }

    basic_string& operator+=(const char* rhs){
        auto len = str_len(rhs);

        ensure_capacity(size() + len + 1);

        std::copy_n(begin() + size(), rhs, len);

        set_size(size() + len);

        (*this)[size()] = '\0';

        return *this;
    }

    basic_string& operator+=(const basic_string& rhs){
        ensure_capacity(size() + rhs.size() + 1);

        std::copy_n(begin() + size(), rhs.begin(), rhs.size());

        set_size(size() + rhs.size());

        (*this)[size()] = '\0';

        return *this;
    }

    //Accessors

    size_t size() const {
        return _size & ~(1UL << 63);
    }

    size_t capacity() const {
        if(is_small()){
            return 16;
        } else {
            return storage.big.capacity;
        }
    }

    bool empty() const {
        return size() == 0;
    }

    CharT* data_ptr(){
        if(is_small()){
            return &storage.small.data[0];
        } else {
            return storage.big.data.get();
        }
    }

    const CharT* data_ptr() const {
        if(is_small()){
            return &storage.small.data[0];
        } else {
            return storage.big.data.get();
        }
    }

    CharT* c_str(){
        return data_ptr();
    }

    const CharT* c_str() const {
        return data_ptr();
    }

    CharT& operator[](size_t i){
        return *(data_ptr() + i);
    }

    const CharT& operator[](size_t i) const {
        return *(data_ptr() + i);
    }

    size_t find(char c) const {
        for(size_t i = 0; i < size(); ++i){
            if((*this)[i] == c){
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
            if((*this)[i] != s[i]){
                return false;
            }
        }

        return true;
    }

    bool operator!=(const CharT* s) const {
        return !(*this == s);
    }

    bool operator==(const basic_string& rhs) const {
        if(size() != rhs.size()){
            return false;
        }

        for(size_t i = 0; i < size(); ++i){
            if((*this)[i] != rhs[i]){
                return false;
            }
        }

        return true;
    }

    bool operator!=(const basic_string& rhs) const {
        return !(*this == rhs);
    }

    //Iterators

    iterator begin(){
        return iterator(data_ptr());
    }

    iterator end(){
        return iterator(data_ptr() + size());
    }

    const_iterator begin() const {
        return const_iterator(data_ptr());
    }

    const_iterator end() const {
        return const_iterator(data_ptr() + size());
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

static_assert(sizeof(string) == 24, "The size of a string must always be 24 bytes");

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

template<typename T>
void to_raw_string(const T& value, char* buffer, size_t n);

template<>
inline void to_raw_string<uint64_t>(const uint64_t& value, char* buffer, size_t n){
    if(n < 20){
        //TODO Print an error ?
        return;
    }

    if(value == 0){
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    std::string s;

    char int_buffer[20];
    int i = 0;
    auto rem = value;

    while(rem != 0){
        int_buffer[i++] = '0' + rem  % 10;
        rem /= 10;
    }

    --i;

    size_t j = 0;
    for(; i >= 0; --i){
        buffer[j++] = int_buffer[i];
    }

    buffer[j] = '\0';
}

template<>
inline void to_raw_string<int64_t>(const int64_t& value, char* buffer, size_t n){
    if(value < 0){
        *buffer = '-';
        to_raw_string(static_cast<uint64_t>(value), buffer + 1, n - 1);
    } else {
        to_raw_string(static_cast<uint64_t>(value), buffer, n);
    }
}

template<>
inline void to_raw_string<uint8_t>(const uint8_t& value, char* buffer, size_t n){
    to_raw_string(static_cast<uint64_t>(value), buffer, n);
}

template<>
inline void to_raw_string<uint16_t>(const uint16_t& value, char* buffer, size_t n){
    to_raw_string(static_cast<uint64_t>(value), buffer, n);
}

template<>
inline void to_raw_string<uint32_t>(const uint32_t& value, char* buffer, size_t n){
    to_raw_string(static_cast<uint64_t>(value), buffer, n);
}

template<>
inline void to_raw_string<int8_t>(const int8_t& value, char* buffer, size_t n){
    to_raw_string(static_cast<uint64_t>(value), buffer, n);
}

template<>
inline void to_raw_string<int16_t>(const int16_t& value, char* buffer, size_t n){
    to_raw_string(static_cast<uint64_t>(value), buffer, n);
}

template<>
inline void to_raw_string<int32_t>(const int32_t& value, char* buffer, size_t n){
    to_raw_string(static_cast<uint64_t>(value), buffer, n);
}

} //end of namespace std

#endif
