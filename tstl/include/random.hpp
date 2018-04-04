//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <types.hpp>

namespace std {

template<typename UIntType, UIntType a, UIntType c, UIntType m>
struct linear_congruential_engine {
    using result_type = UIntType;

    static constexpr const result_type multiplier   = a;
    static constexpr const result_type increment    = c;
    static constexpr const result_type modulus      = m;
    static constexpr const result_type default_seed = 1;

    linear_congruential_engine(result_type seed = default_seed) : seed(seed){
        // Generate the second result directly
        (*this)();
    }

    result_type operator()(){
        return seed = (multiplier * seed + increment) % modulus;
    }

    static constexpr result_type min(){
        return 0;
    }

    static constexpr result_type max(){
        return modulus - 1;
    }

private:
    result_type seed;
};

using mnist_rand0 = linear_congruential_engine<uint32_t, 16807, 0, 2147483647>;
using mnist_rand  = linear_congruential_engine<uint32_t, 48271, 0, 2147483647>;

using default_random_engine = mnist_rand;

template<typename IntType = int>
struct uniform_int_distribution {
    using result_type = IntType;

    uniform_int_distribution(result_type a, result_type b) : a(a), b(b) {
        //Nothing else to init
    }

    template<typename Generator>
    result_type operator()(Generator& g) const {
        static constexpr const auto input_range = Generator::max() - Generator::min();

        const auto output_range = b - a;
        const auto ratio        = input_range / output_range;
        const auto offset       = a - Generator::min();

        return (g() / ratio) + offset;
    }

private:
    const result_type a;
    const result_type b;
};

} //end of namespace std

#endif
