//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STRING_H
#define STRING_H

#include <cstring.hpp>
#include <string_view.hpp>
#include <types.hpp>
#include <algorithms.hpp>
#include <vector.hpp>
#include <unique_ptr.hpp>
#include <iterator.hpp>

namespace std {

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

static constexpr const size_t string_min_capacity = 16;
static constexpr const size_t string_words = string_min_capacity / sizeof(size_t);

//TODO Store size into base and use only one unsigned char for base_short

template<typename CharT>
struct base_short {
    CharT data[string_min_capacity];

    base_short() = default;

    base_short(base_short&) = delete;
    base_short& operator=(base_short&) = delete;

    base_short(base_short&&) = default;
    base_short& operator=(base_short&&) = default;
};

struct base_raw {
    size_t data[string_words];

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

static_assert(string_min_capacity == sizeof(base_short<char>), "base_short must be the correct SSO size");
static_assert(string_min_capacity == sizeof(base_long<char>), "base_long must be the correct SSO size");
static_assert(string_min_capacity == sizeof(base_raw), "base_raw must be the correct SSO size");

/*!
 * \brief A string of the given character type.
 *
 * This implementation uses SSO to not allocate any dynamic memory on short strings (<16 chars)
 */
template<typename CharT>
struct basic_string {
    using iterator = CharT*; ///< The iterator type
    using const_iterator = const CharT*; ///< The const iterator type

    static constexpr const size_t npos = -1;

private:
    using sv_type = std::basic_string_view<CharT>;

    // Utility to ensure valid conversions to string_view
    template <typename T>
    using is_sv = std::integral_constant<bool,
                std::is_convertible<const T&, sv_type>::value
            && !std::is_convertible<const T*, const basic_string*>::value
            && !std::is_convertible<const T&, const CharT*>::value>;

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
        for(size_t i = 0; i < string_words; ++i){
            storage.raw.data[i] = 0;
        }
    }

public:
    //Constructors

    /*!
     * \brief Construct an empty string
     */
    basic_string() : _size(0){
        set_small(true);

        (*this)[0] = '\0';
    }

    /*!
     * \brief Construct a new string from the given raw string
     */
    basic_string(const CharT* s) : _size(str_len(s)) {
        auto capacity = size() + 1;

        set_small(capacity <= 16);

        if(!is_small()){
            new (&storage.big) base_long<CharT>(capacity, new CharT[capacity]);
        }

        std::copy_n(s, capacity, begin());
    }

    /*!
     * \brief Construct a new string of the given capacity.
     */
    explicit basic_string(size_t __capacity) : _size(0) {
        set_small(__capacity <= 16);

        if(!is_small()){
            new (&storage.big) base_long<CharT>(__capacity, new CharT[__capacity]);
        }

        (*this)[0] = '\0';
    }

    /*!
     * \brief Construct a new string from the given range of characters
     */
    template <typename It>
    basic_string(It it, It end) {
        _size = std::distance(it, end);

        auto capacity = size() + 1;

        set_small(capacity <= 16);

        if(!is_small()){
            new (&storage.big) base_long<CharT>(capacity, new CharT[capacity]);
        }

        auto oit = begin();
        while(it != end){
            *oit++ = *it++;
        }

        (*this)[size()] = '\0';
    }

    //Copy

    basic_string(const basic_string& rhs) : _size(rhs._size) {
        if(!is_small()){
            new (&storage.big) base_long<CharT>(size() + 1, new CharT[size() + 1]);
        }

        std::copy_n(rhs.begin(), size() + 1, begin());
    }

    basic_string& operator=(const basic_string& rhs){
        if(this != &rhs){
            return base_assign(rhs);
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

        rhs.set_size(0);
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

            std::copy_n(rhs.begin(), size() + 1, begin());
        }

        rhs.set_size(0);
        rhs.zero();

        return *this;
    }

    // Assign from string_view convertible

    template<typename T, typename = std::enable_if_t<is_sv<T>::value>>
    basic_string& operator=(const T& rhs){
        sv_type sv = rhs;
        return base_assign(sv);
    }

    //Destructors

    /*!
     * \brief Destructs the string and releases all its associated memory
     */
    ~basic_string(){
        if(is_long()){
            storage.big.~base_long();
        }
    }

    //Modifiers

    void adjust_size(size_t size){
        set_size(size);
    }

    /*!
     * \brief Clear the string
     */
    void clear(){
        set_size(0);
        (*this)[0] = '\0';
    }

    /*!
     * \brief Pop the last character of the string.
     */
    void pop_back(){
        set_size(size() - 1);
        (*this)[size()] = '\0';
    }

    /*!
     * \brief Erase the character at the given position
     */
    void erase(size_t position) {
        if(position >= size()){
            return;
        }

        std::copy(begin() + position + 1, end(), begin() + position);

        set_size(size() - 1);
        (*this)[size()] = '\0';
    }

    /*!
     * \brief Ensures a capacity of at least new_capacity
     */
    void reserve(size_t new_capacity){
        ensure_capacity(new_capacity);
    }

    basic_string& append(const basic_string& rhs){
        return base_append(rhs);
    }

    basic_string& append(const char* rhs){
        std::string_view sv = rhs;

        return base_append(sv);
    }

    template<typename T, typename = std::enable_if_t<is_sv<T>::value>>
    basic_string& append(const T& rhs){
        std::string_view sv = rhs;

        return base_append(sv);
    }

    template<typename T>
    basic_string& append(T it, T end){
        return base_append(it, end);
    }

    basic_string& operator+=(const char* rhs){
        return append(rhs);
    }

    basic_string& operator+=(const basic_string& rhs){
        return append(rhs);
    }

    basic_string& operator+=(const sv_type& rhs){
        return append(rhs);
    }

    /*!
     * \brief Creates a string resulting of the concatenation of this string the given char
     */
    basic_string operator+(CharT c) const {
        basic_string copy(*this);

        copy += c;

        return copy;
    }

    /*!
     * \brief Concatenates the given char to the current string
     */
    basic_string& operator+=(CharT c){
        ensure_capacity(size() + 2);

        (*this)[size()] = c;
        (*this)[size() + 1] = '\0';

        set_size(size() + 1);

        return *this;
    }

    //Accessors

    /*!
     * \brief Returns the size of the string
     */
    size_t size() const {
        return _size & ~(1UL << 63);
    }

    /*!
     * \brief Returns the capacity of the string
     */
    size_t capacity() const {
        if(is_small()){
            return 16;
        } else {
            return storage.big.capacity;
        }
    }

    /*
     * \brief Indicates if the string is empty
     */
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

    /*!
     * \return the raw string
     */
    CharT* c_str(){
        return data_ptr();
    }

    /*!
     * \return the raw string
     */
    const CharT* c_str() const {
        return data_ptr();
    }

    /*!
     * \brief Returns a reference to the ith character
     */
    CharT& operator[](size_t i){
        return *(data_ptr() + i);
    }

    /*!
     * \brief Returns a const reference to the ith character
     */
    const CharT& operator[](size_t i) const {
        return *(data_ptr() + i);
    }

    CharT& front() noexcept {
        return data_ptr()[0];
    }

    const CharT& front() const noexcept {
        return data_ptr()[0];
    }

    CharT& back() noexcept {
        return data_ptr()[size() - 1];
    }

    const CharT& back() const noexcept {
        return data_ptr()[size() - 1];
    }

    size_t find(char c, size_t pos = 0) const {
        for(; pos < size(); ++pos){
            if((*this)[pos] == c){
                return pos;
            }
        }

        return npos;
    }

    operator sv_type() const noexcept {
        return {data_ptr(), size()};
    }

    //Iterators

    /*!
     * \brief Returns an iterator to the first character of the string
     */
    iterator begin(){
        return iterator(data_ptr());
    }

    /*!
     * \brief Returns a const iterator to the first character of the string
     */
    const_iterator begin() const {
        return const_iterator(data_ptr());
    }

    /*!
     * \brief Returns an iterator the past-the-end character of the string
     */
    iterator end(){
        return iterator(data_ptr() + size());
    }

    /*!
     * \brief Returns a const iterator the past-the-end character of the string
     */
    const_iterator end() const {
        return const_iterator(data_ptr() + size());
    }

    /*!
     * \brief Lexicographically compare strings
     */
    int compare(const basic_string& rhs) const noexcept {
        return base_compare(rhs);
    }

    /*!
     * \brief Lexicographically compare with string_view rhs
     */
    template<typename T, typename = std::enable_if_t<is_sv<T>::value>>
    int compare(const T& rhs) const noexcept {
        sv_type sv = rhs;

        return base_compare(sv);
    }

    basic_string& assign(basic_string& rhs){
        return base_assign(rhs);
    }

    template<typename T, typename = std::enable_if_t<is_sv<T>::value>>
    basic_string& assign(T& rhs){
        sv_type sv = rhs;
        return base_assign(sv);
    }

    template<typename It>
    basic_string& assign(It it, It end){
        auto n = std::distance(it, end);

        set_size(n);

        ensure_capacity(n + 1, false);

        std::copy(it, end, begin());
        data_ptr()[size()] = '\0';

        return *this;
    }

private:

    void ensure_capacity(size_t new_capacity, bool preserve = true){
        if(new_capacity > 0 && (capacity() < new_capacity)){
            auto new_cap = capacity() * 2;

            if(new_cap < new_capacity){
                new_cap = new_capacity;
            }

            auto new_data = new CharT[new_cap];

            if(preserve){
                std::copy_n(begin(), size() + 1, new_data);
            }

            if(is_small()){
                new (&storage.big) base_long<CharT>(new_cap, new_data);

                set_small(false);
            } else {
                storage.big.data.reset(new_data);
                storage.big.capacity = new_cap;
            }
        }
    }

    template<typename T>
    basic_string& base_append(const T& rhs){
        return base_append(rhs.begin(), rhs.end());
    }

    template<typename T>
    basic_string& base_append(T it, T end){
        auto n = std::distance(it, end);

        ensure_capacity(size() + n + 1);

        std::copy(it, end, begin() + size());

        set_size(size() + n);

        (*this)[size()] = '\0';

        return *this;
    }

    template<typename T>
    basic_string& base_assign(const T& rhs){
        set_size(rhs.size());

        ensure_capacity(rhs.size() + 1, false);

        std::copy(rhs.begin(), rhs.end(), begin());
        data_ptr()[size()] = '\0';

        return *this;
    }

    template<typename T>
    int base_compare(const T& rhs) const noexcept {
        for (size_t i = 0; i < rhs.size() && i < size(); ++i) {
            if ((*this)[i] < rhs[i]) {
                return -1;
            }

            if ((*this)[i] > rhs[i]) {
                return 1;
            }
        }

        if (size() == rhs.size()) {
            return 0;
        }

        if (size() < rhs.size()) {
            return -1;
        }

        return 1;
    }
};

template<typename C>
basic_string<C> operator+(const basic_string<C>& lhs, const basic_string<C>& rhs){
    basic_string<C> result;
    result += lhs;
    result += rhs;
    return result;
}

template<typename C>
basic_string<C> operator+(const C* lhs, const basic_string<C>& rhs){
    basic_string<C> result;
    result += lhs;
    result += rhs;
    return result;
}

template<typename C>
basic_string<C> operator+(const basic_string<C>& lhs, const C* rhs){
    basic_string<C> result;
    result += lhs;
    result += rhs;
    return result;
}

//Operators

/*!
 * \brief Test if two std::basic_string are equal to each other
 */
template <typename CharT>
bool operator==(const basic_string<CharT>& x, const basic_string<CharT>& y){
    return x.compare(y) == 0;
}

/*!
 * \brief Test if one std::basic_string and one raw string are equal to each other
 */
template <typename CharT>
bool operator==(const basic_string<CharT>& x, const CharT* y){
    return x.compare(y) == 0;
}

/*!
 * \brief Test if one std::basic_string and one raw string are equal to each other
 */
template <typename CharT>
bool operator==(const CharT* x, const basic_string<CharT>& y){
    return y.compare(x) == 0;
}

/*!
 * \brief Test if two std::basic_string are not equal to each other
 */
template <typename CharT>
bool operator!=(const basic_string<CharT>& x, const basic_string<CharT>& y) {
    return x.compare(y) != 0;
}

/*!
 * \brief Test if one std::basic_string and one raw string are not equal to each other
 */
template <typename CharT>
bool operator!=(const basic_string<CharT>& x, const CharT* y) {
    return x.compare(y) != 0;
}

/*!
 * \brief Test if one std::basic_string and one raw string are not equal to each other
 */
template <typename CharT>
bool operator!=(const CharT* x, const basic_string<CharT>& y) {
    return y.compare(x) != 0;
}

template <typename CharT>
bool operator<(const basic_string<CharT>& x, const basic_string<CharT>& y){
    return x.compare(y) < 0;
}

template <typename CharT>
bool operator<(const basic_string<CharT>& x, const CharT* y){
    return x.compare(y) < 0;
}

template <typename CharT>
bool operator<(const CharT* x, const basic_string<CharT>& y){
    return y.compare(x) > 0;
}

template <typename CharT>
bool operator>(const basic_string<CharT>& x, const basic_string<CharT>& y){
    return x.compare(y) > 0;
}

template <typename CharT>
bool operator>(const basic_string<CharT>& x, const CharT* y){
    return x.compare(y) > 0;
}

template <typename CharT>
bool operator>(const CharT* x, const basic_string<CharT>& y){
    return y.compare(x) < 0;
}

template <typename CharT>
bool operator<=(const basic_string<CharT>& x, const basic_string<CharT>& y){
    return x.compare(y) <= 0;
}

template <typename CharT>
bool operator<=(const basic_string<CharT>& x, const CharT* y){
    return x.compare(y) <= 0;
}

template <typename CharT>
bool operator<=(const CharT* x, const basic_string<CharT>& y){
    return y.compare(x) >= 0;
}

template <typename CharT>
bool operator>=(const basic_string<CharT>& x, const basic_string<CharT>& y){
    return x.compare(y) >= 0;
}

template <typename CharT>
bool operator>=(const basic_string<CharT>& x, const CharT* y){
    return x.compare(y) >= 0;
}

template <typename CharT>
bool operator>=(const CharT* x, const basic_string<CharT>& y){
    return y.compare(x) <= 0;
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

    return parts;
}

template<typename Char>
void split_append(const std::basic_string<Char>& s, std::vector<std::basic_string<Char>>& container, char sep = ' '){
    std::basic_string<Char> current(s.size());

    for(char c : s){
        if(c == sep && !current.empty()){
            container.push_back(current);
            current.clear();
        } else if(c == sep){
            continue;
        } else {
            current += c;
        }
    }

    if(!current.empty()){
        container.push_back(current);
    }
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

    return s;
}

template<>
inline std::string to_string<int64_t>(const int64_t& value){
    if(value < 0){
        std::string s("-");
        s += to_string(static_cast<uint64_t>(-value));
        return s;
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
        to_raw_string(static_cast<uint64_t>(-value), buffer + 1, n - 1);
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

template<typename T>
inline uint64_t atoui(const basic_string_view<T>& s){
    uint64_t value = 0;
    uint64_t mul = 1;

    for(size_t i = s.size(); i > 0; --i){
        auto c = s[i - 1];

        if(c < '0' || c  > '9'){
            return value;
        }

        value += mul * (c - '0');

        mul *= 10;
    }

    return value;
}

inline uint64_t atoui(const std::string& s){
    std::string_view sv = s;
    return atoui(sv);
}

} //end of namespace std

#endif
