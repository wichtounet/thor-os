//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <type_traits.hpp>
#include <utility.hpp>
#include <types.hpp>
#include <enable_if.hpp>

namespace std {

/*!
 * \brief Copies all elements in the range [first, last).
 *
 * The behavior is undefined if d_first is within the range [first, last)
 *
 * \param first The beginning of the range
 * \param last The end of the range
 * \param d_first The output iterator
 */
template<typename InputIterator, typename OutputIterator>
void copy(InputIterator first, InputIterator last, OutputIterator d_first){
    if(first != last){
        *d_first = *first;

        while(++first != last){
            *++d_first = *first;
        }
    }
}

/*!
 * \brief Copy bytes bytes of memory from in to out
 * \param out Pointer to the output
 * \param in Pointer to the input
 * \param bytes The number of bytes
 */
inline void memcpy(char* out, const char* in, size_t bytes){
    if(!bytes){
        return;
    }

    // Copy as much as possible 64 bits at at time
    if(bytes >= 8){
        auto* out64 = reinterpret_cast<uint64_t*>(out);
        auto* in64 = reinterpret_cast<const uint64_t*>(in);

        const size_t l = bytes / 8;

        for(size_t i = 0; i < l; ++i){
            out64[i] = in64[i];
        }

        bytes -= l * 8;
        out += l * 8;
        in += l * 8;
    }

    // Finish up byte by byte
    while(bytes >= 1){
        *out++ = *in++;
        --bytes;
    }
}

/*!
 * \brief Copies exactly count values from the range beginning at first to the range beginning at result, if count>0. Does nothing otherwise.
 * \param first The beginning of the input range
 * \param count The number fo elements to copy
 * \param out The output iterator
 */
template<typename InputIterator, typename OutputIterator, std::enable_if_t<!std::has_trivial_assign<typename std::iterator_traits<OutputIterator>::value_type>::value, int> = 42>
void copy_n(InputIterator first, size_t count, OutputIterator out){
    if(count > 0){
        *out = *first;

        while(--count){
            *++out = *++first;
        }
    }
}

/*!
 * \brief Copies exactly count values from the range beginning at first to the range beginning at result, if count>0. Does nothing otherwise.
 * \param first The beginning of the input range
 * \param count The number fo elements to copy
 * \param out The output iterator
 */
template<typename InputIterator, typename OutputIterator, std::enable_if_t<std::has_trivial_assign<typename std::iterator_traits<OutputIterator>::value_type>::value, int> = 42>
void copy_n(InputIterator first, size_t count, OutputIterator out){
    memcpy(reinterpret_cast<char*>(out), reinterpret_cast<const char*>(first), count * sizeof(decltype(*out)));
}

/*!
 * \brief Moves exactly count values from the range beginning at first to the range beginning at result, if count>0. Does nothing otherwise.
 * \param first The beginning of the input range
 * \param count The number fo elements to move
 * \param out The output iterator
 */
template<typename InputIterator, typename OutputIterator>
void move_n(InputIterator in, size_t n, OutputIterator out){
    if(n > 0){
        *out = std::move(*in);

        while(--n){
            *++out = std::move(*++in);
        }
    }
}

/*!
 * \brief Clear bytes bytes of memory from out
 * \param out Pointer to the output
 * \param bytes The number of bytes
 */
inline void memclr(char* out, size_t bytes){
    if(!bytes){
        return;
    }

    // Copy as much as possible 64 bits at at time
    if(bytes >= 8){
        auto* out64 = reinterpret_cast<uint64_t*>(out);

        const size_t l = bytes / 8;

        for(size_t i = 0; i < l; ++i){
            out64[i] = 0;
        }

        bytes -= l * 8;
        out += l * 8;
    }

    // Finish up byte by byte
    while(bytes >= 1){
        *out++ = 0;
        --bytes;
    }
}

/*!
 * \brief Assigns the given value to the elements in the range [first, last).
 * \param first The beginning of the range
 * \param last The end of the range
 * \param value The value to write
 */
template<typename ForwardIterator, typename T>
void fill(ForwardIterator first, ForwardIterator last, const T& value){
    if(first != last){
        *first = value;

        while(++first != last){
            *first = value;
        }
    }
}

/*!
 * \brief Assigns the given value to the first count elements in the range beginning at
 * first if count > 0. Does nothing otherwise.
 * \param first The beginning of the range
 * \param count The number of elements
 * \param value The value to write
 */
template<typename ForwardIterator, typename T, std::enable_if_t<!(std::is_integral<typename std::iterator_traits<ForwardIterator>::value_type>::value && is_integral<T>::value), int> = 42>
void fill_n(ForwardIterator first, size_t count, const T& value){
    if(count > 0){
        *first = value;

        while(--count){
            *++first = value;
        }
    }
}

/*!
 * \brief Assigns the given value to the first count elements in the range beginning at
 * first if count > 0. Does nothing otherwise.
 * \param first The beginning of the range
 * \param count The number of elements
 * \param value The value to write
 */
template<typename ForwardIterator, typename T, std::enable_if_t<std::is_integral<typename std::iterator_traits<ForwardIterator>::value_type>::value && is_integral<T>::value, int> = 42>
void fill_n(ForwardIterator first, size_t count, const T& value){
    // TODO This is definitely not good, should properly compare to zero
    bool v{value};
    if(!v){
        memclr(reinterpret_cast<char*>(first), count * sizeof(decltype(*first)));
    } else {
        if(count > 0){
            *first = value;

            while(--count){
                *++first = value;
            }
        }
    }
}

template<typename Iterator1, typename Iterator2>
size_t compare_n(Iterator1 it1, Iterator2 it2, size_t count){
    if(count > 0){
        while(count--){
            if(*it1 != *it2){
                return *it1- *it2;
            } else {
                ++it1;
                ++it2;
            }
        }
    }

    return 0;
}

template<typename Iterator1, typename Iterator2>
bool equal_n(Iterator1 it1, Iterator2 it2, size_t n){
    return compare_n(it1, it2, n) == 0;
}

template<typename Iterator, typename Functor>
void for_each(Iterator it, Iterator end, Functor func){
    while(it != end){
        func(*it);

        ++it;
    }
}

template<typename Iterator, typename T>
T accumulate(Iterator it, Iterator end, T init){
    while(it != end){
        init = init + *it;

        ++it;
    }

    return init;
}

template<typename Iterator, typename T>
Iterator find(Iterator first, Iterator last, const T& value){
    for (; first != last; ++first) {
        if (*first == value) {
            return first;
        }
    }

    return last;
}

template <typename Iterator, typename Pred>
Iterator find_if(Iterator first, Iterator last, Pred pred) {
    for (; first != last; ++first) {
        if (pred(*first)) {
            return first;
        }
    }

    return last;
}

template< typename Iterator, typename T >
Iterator remove(Iterator first, Iterator last, const T& value){
    first = std::find(first, last, value);

    if (first != last){
        for(auto it = first; ++it != last; ){
            if (!(*it == value)){
                *first++ = std::move(*it);
            }
        }
    }

    return first;
}

template <typename Iterator, typename Pred>
Iterator remove_if(Iterator first, Iterator last, Pred pred) {
    first = std::find_if(first, last, pred);

    if (first != last) {
        for (auto it = first; ++it != last;) {
            if (!pred(*it)) {
                *first++ = std::move(*it);
            }
        }
    }

    return first;
}

template<typename T>
constexpr const T& min(const T& a, const T& b){
    return a <= b ? a : b;
}

template<typename T>
constexpr const T& max(const T& a, const T& b){
    return a >= b ? a : b;
}

template<typename T>
constexpr const T& clip(const T& a, const T& min, const T& max){
    return (a < min) ? min : (a > max) ? max : a;
}

} //end of namespace std

#endif
