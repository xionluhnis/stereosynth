/* 
 * File:   gaussian.h
 * Author: akaspar
 *
 * Created on November 17, 2014, 8:35 PM
 */

#ifndef GAUSSIAN_H
#define	GAUSSIAN_H

#include "rng.h"

namespace pm {
    
    /// Gaussian Noise
	template <typename T>
	inline T gaussian(RNG rand, T sigma, bool noStore = false) {
		// Box-Muller transform
		// @see http://projecteuclid.org/DPubS?verb=Display&version=1.0&service=UI&handle=euclid.aoms/1177706645&page=record
		// @see http://en.wikipedia.org/wiki/Box-Muller_transform
		
		// the last instances
		static T u1; // radius
		static T u2; // angle
		static bool hasSpare = false; // whether we have only used them once

		// reuse if they're fresh
		if(hasSpare){
			hasSpare = false;
			return sigma * std::sqrt(u1) * std::sin(u2);
		}

		// renew data
		u1 = rand();
		if(u1 < 1e-100) u1 = 1e-100;
		u1 = -2 * std::log(u1); // squared radius, inv-exponentially distributed
		u2 = rand() * M_PI * 2; // angle, uniformly distributed over [0;2pi]

		// available for afterwards
		hasSpare = !noStore; //< unless we asked not to keep it
		// gaussian version
		return sigma * std::sqrt(u1) * std::cos(u2);
	}
	
	template <typename Point, typename T>
	inline Point gaussian2d(RNG rand, T sigma) {
		// squared radius, inv-exponentially distributed
		T u1 = rand();
		if(u1 < 1e-100) u1 = 1e-100;
		u1 = -2 * std::log(u1);
		
		// angle, uniformly distributed over [0;2pi]
		T u2 = rand() * M_PI * 2;
		
		// our sample
		T r_s = std::sqrt(u1) * sigma;
		return Point(r_s * std::cos(u2), r_s * std::sin(u2)); 
	}
    
}

#endif	/* GAUSSIAN_H */

