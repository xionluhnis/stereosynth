/* 
 * File:   uniform.h
 * Author: akaspar
 *
 * Created on November 17, 2014, 8:33 PM
 */

#ifndef UNIFORM_H
#define	UNIFORM_H

#include "rng.h"
#include "../math/vec.h"

namespace pm {
    
    enum UniformType {
        Discrete = 0,
        Continuous = 1,
        Class = 2,
        Unknown = -1
    };
    template <typename T>
    struct uniform_type {
        enum {
            type = std::is_class<T>::value ?        Class :
                std::is_integral<T>::value ?        Discrete :
                std::is_floating_point<T>::value ?  Continuous : 
                                                    Unknown
        };
    };
    
    template <typename T, int type>
    struct uniform_impl {
        static T get(RNG rand, T a, T b);
    };
    
    template <typename T>
    inline T uniform(RNG rand, T a, T b) {
        return uniform_impl<T, uniform_type<T>::type>::get(rand, a, b);
    }
    
    /// Continuous Uniform RV
    template <typename T>
    struct uniform_impl< T, Continuous > {
        inline static T get(RNG rand, T a, T b){
            // assert(a <= b && "Continuous uniform between inverted bounds");
            return a + (b - a) * rand();
        }
    };
    
    /// Discrete Uniform RV ~ Unif{a .. b}
    template <typename T>
	struct uniform_impl< T, Discrete > {
        inline static T get(RNG rand, T a, T b) {
            // assert(a <= b && "Discrete uniform between inverted bounds");
            T r = a + std::floor(rand() * (b - a + 1));
            if (r > b) return b;
            return r;
        }
    };
    
    /// Vector versions RV ~ (Unif[a,b])^cn
    template <typename E, int cn>
    struct uniform_impl< Vec<E, cn>, Class > {
        typedef Vec<E, cn> vec;
        
        inline static vec get(RNG rand, const vec &a, const vec &b) {
            vec v;
            for(int i = 0; i < cn; ++i){
                v[i] = uniform<E>(rand, a[i], b[i]);
            }
            return v;
        }
    };
    
}

#endif	/* UNIFORM_H */

