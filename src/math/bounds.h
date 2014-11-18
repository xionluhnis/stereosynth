/* 
 * File:   bounds.h
 * Author: akaspar
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
        
        Bounds() {}
        Bounds(const vec &v1, const vec &v2) : min(v1), max(v2) {}
        
        static Bounds get(const vec &p1, const vec &p2) {
            Bounds bounds;
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
        
        static Bounds get(const std::vector<vec> &p) {
            Bounds bounds;
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
        
        inline vec range() const {
            return max - min;
        }
    };
    
}

#endif	/* BOUNDS_H */

