/* 
 * File:   rng.h
 * Author: akaspar
 *
 * Created on November 17, 2014, 8:32 PM
 */

#ifndef SAMPLING_RNG_H
#define	SAMPLING_RNG_H

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>

namespace pm {
	// Random Number Generator
	typedef float (*RNG)(void);

    /**
     * Simple uniform variable in the continuous interval [0;1]
     */
	float unif01(void) {
		return float(rand()) / float(RAND_MAX);
	}
	
	/**
	 * \brief Initialize the RNG with a given seed
	 * 
     * \param s the seed to use
     */
	inline void seed(unsigned int s) {
		srand(s);
	}
	
	/**
	 * \brief Initialize the RNG with the current time (seconds!)
     */
	inline void seed() {
		seed(time(NULL));
	}
    
    /**
     * Return the current temporal seed
     */
    inline unsigned int timeSeed() {
        return time(NULL);
    }

	/// Discrete Bernoulli RV ~ Bernoulli(p)

	inline bool bernoulli(RNG rand, float p = 0.5f) {
		return rand() <= p;
	}
	
}

#endif	/* SAMPLING_RNG_H */

