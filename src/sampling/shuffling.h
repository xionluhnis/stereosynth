/* 
 * File:   shuffling.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 17, 2014, 8:34 PM
 */

#ifndef SHUFFLING_H
#define	SHUFFLING_H

#include "rng.h"

namespace pm {
    
    /**
	 * \brief Knuth in-place shuffling
	 * 
	 * \param r
	 *			the random number source
	 * \param index
	 *			the index to shuffle
	 * \param N
	 *			the size of the index
	 * \see http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
	 * \note randomness is limited as the PRNG is now sophisticated, 
	 * thus the shuffle will be bad and biased, but it should be enough ...
	 */
	template <typename T>
	inline void knuth_shuffle(RNG r, T *index, int N) {
		for (int i = N - 1; i > 0; --i) {
			int j = uniform(r, 0, i);
			std::swap(index[j], index[i]);
		}
	}
    
}

#endif	/* SHUFFLING_H */

