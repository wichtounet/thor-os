//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PAIR_H
#define PAIR_H

namespace std {

/*!
 * \brief Simply container to hold a pair of element
 */
template<typename T1, typename T2>
class pair {
public:
    using first_type = T1;  ///< The type of the first element
    using second_type = T2; ///< The type of the second element

    first_type first;   ///< The first element
    second_type second; ///< The second element

    //Constructor

    constexpr pair(const first_type& a, const second_type& b) : first(a), second(b){
        //Nothing to init
    }

    template<typename U1, typename U2>
    constexpr pair(U1&& x, U2&& y) : first(std::forward<U1>(x)), second(std::forward<U2>(y)){
        //Nothing to init
    }

    //Copy constructors

    constexpr pair(const pair&) = default;

    template<typename U1, typename U2>
    constexpr pair(const pair<U1, U2>& p) : first(p.first), second(p.second) {
        //Nothing to init
    }

    template<typename U1, typename U2>
    pair& operator=(const pair<U1, U2>& p){
        first = p.first;
        second = p.second;
        return *this;
    }

    //Move constructors

    constexpr pair(pair&&) = default;

    template<typename U1, typename U2>
    constexpr pair(pair<U1, U2>&& p) : first(std::move(p.first)), second(std::move(p.second)) {
        //Nothing to init
    }

    template<typename U1, typename U2>
    pair& operator=(pair<U1, U2>&& p){
        first = std::forward<U1>(p.first);
        second = std::forward<U2>(p.second);
        return *this;
    }
};

/*!
 * \brief Helper to construct a pair
 */
template<typename T1, typename T2>
inline constexpr pair<T1, T2> make_pair(T1&& x, T2&& y){
    return pair<T1, T2>(std::forward<T1>(x), std::forward<T2>(y));
}

} //end of namespace std

#endif
