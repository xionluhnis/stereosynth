/* 
 * File:   bounds.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 17, 2014, 7:22 PM
 */

#ifndef BOUNDS_H
#define	BOUNDS_H

#include "vec.h"

namespace pm {
    
    template <typename T, int numDim>
    struct Bounds {
        typedef Vec<T, numDim> vec;
        typedef Bounds<T, numDim> bounds;
        
        union {
            vec min;
            vec start;
            vec from;
        };
        
        union {
            vec max;
            vec end;
            vec to;
        };
        
		// default constructor has no value!
        Bounds() {}
		// initialization constructor
		explicit Bounds(T init) {
			for(int i = 0; i < numDim; ++i){
				min[i] = init;
				max[i] = init;
			}
		}
        Bounds(const vec &v1, const vec &v2) : min(v1), max(v2) {}
        Bounds(const vec &c, T radius) {
            for(int i = 0; i < numDim; ++i){
                min[i] = c[i] - radius;
                max[i] = c[i] + radius;
            }
        }
        
        bounds operator &(const bounds &b) const {
            bounds newBounds;
            for(int i = 0; i < numDim; ++i){
                newBounds.min[i] = std::max(min[i], b.min[i]);
                newBounds.max[i] = std::min(max[i], b.max[i]);
            }
            return newBounds;
        }
        
        bounds operator |(const bounds &b) const {
            bounds newBounds;
            for(int i = 0; i < numDim; ++i){
                newBounds.min[i] = std::min(min[i], b.min[i]);
                newBounds.max[i] = std::max(max[i], b.max[i]);
            }
            return newBounds;
        }
        
        static bounds get(const vec &p1, const vec &p2) {
            bounds bounds;
            for(int i = 0; i < numDim; ++i) {
                if(p1[i] < p2[i]){
                    bounds.min[i] = p1[i];
                    bounds.max[i] = p2[i];
                } else {
                    bounds.min[i] = p2[i];
                    bounds.max[i] = p1[i];
                }
            }
        }
        
        static bounds get(const std::vector<vec> &p) {
            bounds bounds;
            for(int i = 0; i < numDim; ++i) {
                bounds.min[i] = p[0][i];
                bounds.max[i] = p[0][i];
                for(auto it = ++p.begin(); it != p.end(); ++it){
                    const vec &v = *it;
                    if(v[i] < bounds.min[i]) bounds.min[i] = v[i];
                    /* else */ if(v[i] > bounds.max[i]) bounds.max[i] = v[i];
                }
            }
        }
        
        inline bool empty() const {
            bool singular = true;
            for(int i = 0; i < numDim; ++i){
                if(min[i] > max[i]){
                    return true;
                } else if(min[i] < max[i]){
                    singular = false;
                }
                // only singular if min==max
            }
            return singular;
        }
        
        inline vec range() const {
            return max - min;
        }
        
        inline bool contains(const vec &v) const {
            for(int i = 0; i < numDim; ++i){
                if(v[i] < min[i] || v[i] > max[i])
                    return false;
            }
            return true;
        }
		
		inline bool operator ==(const bounds &b) const {
			return min == b.min && max == b.max;
		}
		inline bool operator !=(const bounds &b) const {
			return !(*this == b);
		}
    };
    
    typedef Bounds<int, 2> Bounds2i;
    typedef Bounds<float, 2> Bounds2f;
    
}

#endif	/* BOUNDS_H */

