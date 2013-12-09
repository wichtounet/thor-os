#ifndef PAIR_H
#define PAIR_H

template<typename T1, typename T2>
class pair {
public:
    typedef T1 first_type;
    typedef T2 second_type;

    first_type first;
    second_type second;

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

template<typename T1, typename T2>
inline constexpr pair<T1, T2> make_pair(T1&& x, T2&& y){
    return pair<T1, T2>(std::forward<T1>(x), std::forward<T2>(y));
}

#endif
