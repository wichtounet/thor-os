//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <cstring.hpp>
#include <types.hpp>
#include <algorithms.hpp>
#include <iterator.hpp>

namespace std {

template<typename CharT>
struct basic_string_view {
    using value_type      = CharT;             ///< The value type contained in the vector
    using pointer         = value_type*;       ///< The pointer type contained in the vector
    using const_pointer   = const value_type*; ///< The pointer type contained in the vector
    using reference       = value_type&;       ///< The pointer type contained in the vector
    using const_reference = const value_type&; ///< The pointer type contained in the vector
    using size_type       = size_t;            ///< The size type
    using iterator        = value_type*;       ///< The iterator type
    using const_iterator  = const value_type*; ///< The const iterator type

    using reverse_iterator       = std::reverse_iterator<iterator>;       ///< The reverse iterator type
    using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< The const reverse iterator type

    static constexpr const size_t npos = -1;

    constexpr basic_string_view() noexcept
            : _data(nullptr), _size(0) {
        // Nothing else to init
    }

    constexpr basic_string_view(const basic_string_view&) noexcept = default;
    basic_string_view& operator=(const basic_string_view&) noexcept = default;

    constexpr basic_string_view(const CharT* str)
            : _data(str), _size(str_len(str)) {
        // Nothing else to init
    }

    constexpr basic_string_view(const CharT* str, size_t len)
            : _data(str), _size(len) {
        // Nothing else to init
    }

    // Iterator support

    constexpr const_iterator begin() const noexcept {
        return _data;
    }

    constexpr const_iterator end() const noexcept {
        return _data + _size;
    }

    constexpr const_iterator cbegin() const noexcept {
        return _data;
    }

    constexpr const_iterator cend() const noexcept {
        return _data + _size;
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(&_data[int64_t(_size) - 1]);
    }

    const_reverse_iterator rend() const noexcept {
        return reverse_iterator(&_data[-1]);
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(&_data[int64_t(_size) - 1]);
    }

    const_reverse_iterator crend() const noexcept {
        return reverse_iterator(&_data[-1]);
    }

    // capacity

    constexpr size_t size() const noexcept {
        return _size;
    }

    constexpr size_t length() const noexcept {
        return _size;
    }

    constexpr size_t max_size() const noexcept {
        return 18446744073709551615UL;
    }

    constexpr bool empty() const noexcept {
        return !_size;
    }

    // element access

    constexpr const_reference operator[](size_t pos) const {
        return _data[pos];
    }

    constexpr const_reference at(size_t pos) const {
        return _data[pos];
    }

    constexpr const_reference front() const {
        return _data[0];
    }

    constexpr const_reference back() const {
        return _data[_size - 1];
    }

    constexpr const_pointer data() const noexcept {
        return _data;
    }

    // modifiers

    // TODO Once we go in c++14, add  constexpr to these three functions

    void remove_prefix(size_t n){
        _data += n;
        _size -= n;
    }

    void remove_suffix(size_t n){
        _size -= n;
    }

    void swap(basic_string_view& s) noexcept{
        std::swap(_size, s._size);
        std::swap(_data, s._data);
    }

    constexpr basic_string_view substr(size_t pos = 0, size_t n = npos) const {
        return {data + pos, std::min(n, size() - pos)};
    }

    // Compare

    int compare(basic_string_view& rhs) const noexcept {
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

private:
    const CharT * _data;
    size_t _size;
};

using string_view = basic_string_view<char>;

static_assert(sizeof(string_view) == 16, "The size of a string_view must always be 16 bytes");

// TODO Switch to C++14 and use constexpr for these functions

// Note: The identity_of_t trick is simply to ensure that template argument
// deduction is only done on one of the two arguments. This is allowing implicit
// conversion (string and string_view for instance)

// non-member comparison functions

template <typename CharT>
bool operator==(basic_string_view<CharT> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) == 0;
}

template <typename CharT>
bool operator==(basic_string_view<CharT> x, std::identity_of_t<basic_string_view<CharT>> y) noexcept {
    return x.compare(y) == 0;
}

template <typename CharT>
bool operator==(std::identity_of_t<basic_string_view<CharT>> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) == 0;
}

template <typename CharT>
bool operator!=(basic_string_view<CharT> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) != 0;
}

template <typename CharT>
bool operator!=(std::identity_of_t<basic_string_view<CharT>> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) != 0;
}

template <typename CharT>
bool operator!=(basic_string_view<CharT> x, std::identity_of_t<basic_string_view<CharT>> y) noexcept {
    return x.compare(y) != 0;
}

template <typename CharT>
bool operator<(basic_string_view<CharT> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) < 0;
}

template <typename CharT>
bool operator<(std::identity_of_t<basic_string_view<CharT>> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) < 0;
}

template <typename CharT>
bool operator<(basic_string_view<CharT> x, std::identity_of_t<basic_string_view<CharT>> y) noexcept {
    return x.compare(y) < 0;
}

template <typename CharT>
bool operator>(basic_string_view<CharT> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) > 0;
}

template <typename CharT>
bool operator>(std::identity_of_t<basic_string_view<CharT>> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) > 0;
}

template <typename CharT>
bool operator>(basic_string_view<CharT> x, std::identity_of_t<basic_string_view<CharT>> y) noexcept {
    return x.compare(y) > 0;
}

template <typename CharT>
bool operator<=(basic_string_view<CharT> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) <= 0;
}

template <typename CharT>
bool operator<=(std::identity_of_t<basic_string_view<CharT>> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) <= 0;
}

template <typename CharT>
bool operator<=(basic_string_view<CharT> x, std::identity_of_t<basic_string_view<CharT>> y) noexcept {
    return x.compare(y) <= 0;
}

template <typename CharT>
bool operator>=(basic_string_view<CharT> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) >= 0;
}

template <typename CharT>
bool operator>=(std::identity_of_t<basic_string_view<CharT>> x, basic_string_view<CharT> y) noexcept {
    return x.compare(y) >= 0;
}

template <typename CharT>
bool operator>=(basic_string_view<CharT> x, std::identity_of_t<basic_string_view<CharT>> y) noexcept {
    return x.compare(y) >= 0;
}

} //end of namespace std

#endif
