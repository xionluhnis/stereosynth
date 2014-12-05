/* 
 * File:   pointx.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 5, 2014, 2:22 PM
 */

#ifndef POINTX_H
#define	POINTX_H

#include "../math/point.h"

namespace pm {
    
    template< typename T >
	struct IndexedPoint : public Point<T> {
		typedef T scalar;
		typedef Vec<T, 3> vec;
		typedef Point<T> base;
		typedef IndexedPoint<T> point;

		enum {
			typeDepth = DataDepth<scalar>::value,
			channels = 3,
			type = IM_MAKETYPE(typeDepth, channels)
		};

		IndexedPoint() : Point<T>(), depth(0) {}
		IndexedPoint(T a, T b, int d) : Point<T>(a, b), depth(d) {}
		IndexedPoint(base p, int d) : Point<T>(p.x, p.y), depth(d) {}
		
		inline point operator +(const point &p) const {
			return point(base::x + p.x, base::y + p.y, depth);
		}
		inline point operator -(const point &p) const {
			return point(base::x - p.x, base::y - p.y, depth);
		}
		inline point operator *(T f) const {
			return point(base::x * f, base::y * f, depth);
		}
		inline point operator -() const {
			return point(-base::x, -base::y, depth);
		}
		inline operator base() const {
			return base(base::x, base::y);
		}
        
        inline operator vec() const {
            return vec(x, y, z);
        }
		
        union {
            int depth;
            int index;
            int z;
        };
	};
    
    typedef IndexedPoint<int> Point2ix;
    typedef IndexedPoint<float> Point2fx;
    typedef IndexedPoint<double> Point3dx;
    
}

#endif	/* POINTX_H */

