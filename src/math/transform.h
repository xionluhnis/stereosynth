/* 
 * File:   transform.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 17, 2014, 7:08 PM
 */

#ifndef TRANSFORM_H
#define	TRANSFORM_H

#include "point.h"

namespace pm {
    
    template <typename T>
    struct Transform {
        T transform(const T &) const;
        
        inline T operator %(const T &p) const {
            return transform(p);
        }
    };
    
    template <typename T1, typename T2 = T1>
    struct Translation : public T1, public Transform<T2> {
        typedef typename T1::scalar scalar;
        
        T1 transform(const T2 &p) const {
            return T1(p) + *this;
        }
        explicit Translation(scalar x = 0, scalar y = 0) : T1(x, y){}
        Translation(const T1 &t) : T1(t){}
    };
    
    template <typename T>
    struct AffineTransform : public Transform<T> {
        typedef typename T::scalar scalar;
        
        Translation<T> t;
        scalar angle;
        scalar scaleX;
        scalar scaleY;
        
        T transform(const T &p) const;
        
        AffineTransform(const Translation<T> &tr, scalar ang = 0, scalar sx = 1, scalar sy = 1)
            : t(tr), angle(ang), scaleX(sx), scaleY(sy) {}
        AffineTransform() : angle(0), scaleX(1), scaleY(1) {}
    };
    
    template<typename S>
	struct AffineTransform< Point<S> > : public Transform< Point<S> > {
		typedef Point<S> point;
		typedef typename point::scalar scalar;
        
        Translation<point> t;
        scalar angle;
        scalar scaleX;
        scalar scaleY;
        
        point transform(const point &p) const {
			point q = p;
			q.x *= scaleX;
			q.y *= scaleY;

			scalar cosA = std::cos(angle);
			scalar sinA = std::sin(angle);

			scalar tx = cosA * q.x - sinA * q.y;
			scalar ty = sinA * q.x + cosA * q.y;
			return point(tx, ty) + t;
		}
        
        AffineTransform(const Translation<point> &tr, scalar ang = 0, scalar sx = 1, scalar sy = 1)
            : t(tr), angle(ang), scaleX(sx), scaleY(sy) {}
        AffineTransform() : angle(0), scaleX(1), scaleY(1) {}
	};
    
}

#endif	/* TRANSFORM_H */

