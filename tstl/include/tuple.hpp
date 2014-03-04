//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TUPLE_H
#define TUPLE_H

#include<types.hpp>

namespace std {

/*
 Base class used to store the head of the tuple impl. If
 the head type is an empty type, use EBO to optimize size
*/

template<size_t I, typename H, bool Empty>
struct head_base;

template<size_t I, typename H>
struct head_base<I, H, true> : H {
	head_base() : H(){}
	head_base(const H& h) : H(h){}

	static H& head(head_base& b){ return b; }
	static const H& head(const head_base& b){ return b; }
};

template<size_t I, typename H>
struct head_base<I, H, false> {
	H head_impl;

	head_base() : head_impl(){}
	head_base(const H& h) : head_impl(h){}

	static H& head(head_base& b){ return b.head_impl; }
	static const H& head(const head_base& b){ return b.head_impl; }
};

/* Tuple implementation */

template<size_t I, typename... E>
struct tuple_impl;

template<size_t I>
struct tuple_impl<I> {};

template<size_t I, typename H, typename... E>
struct tuple_impl<I, H, E...> : public tuple_impl<I + 1, E...>, private head_base<I, H, __is_empty(H)> {
	typedef tuple_impl<I + 1, E...> parent_t;
	typedef head_base<I, H, __is_empty(H)> base_parent_t;

	tuple_impl() : parent_t(), base_parent_t() {}

	explicit tuple_impl(const H& head, const E&... elements): parent_t(elements...), base_parent_t(head) {}

	static H& head(tuple_impl& t){ return base_parent_t::head(t); }
	static const H& head(const tuple_impl& t){ return base_parent_t::head(t); }
};

/* tuple class */

template<typename... E>
struct tuple : tuple_impl<0, E...> {
	typedef tuple_impl<0, E...> parent_t;

	tuple() : parent_t(){}

	explicit tuple(const E&... elements): parent_t(elements...) {}
};

/* Helper to get the type of a tuple */

template<size_t I, typename T>
struct tuple_element;

template<size_t I, typename H, typename... T>
struct tuple_element<I, tuple<H, T...>> : tuple_element<I - 1, tuple<T...>> { };

template<typename H, typename... T>
struct tuple_element<0, tuple<H, T...>> {
	typedef H type;
};

template<int I, typename H, typename... T>
inline H& get_helper(tuple_impl<I, H, T...>& t){
	return tuple_impl<I, H, T...>::head(t);
}

template<int I, typename H, typename... T>
inline const H& get_helper(const tuple_impl<I, H, T...>& t) {
	return tuple_impl<I, H, T...>::head(t);
}

template<int I, typename... E>
inline typename tuple_element<I, tuple<E...> >::type& get(tuple<E...>& t){
	return get_helper<I>(t);
}

template<int I, typename... E>
inline const typename tuple_element<I, tuple<E...> >::type get(const tuple<E...>& t){
	return get_helper<I>(t);
}

template<typename... E>
inline tuple<E...> make_tuple(E... args){
	return tuple<E...>(args...);
}

} //end of namespace std

#endif
