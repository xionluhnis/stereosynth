/* 
 * File:   transform.h
 * Author: akaspar
 *
 * Created on November 17, 2014, 7:08 PM
 */

#ifndef TRANSFORM_H
#define	TRANSFORM_H

#include "point.h"

namespace pm {
    
    template <typename T>
    struct Transform {
        virtual T transform(const T &) const = 0;
        
        inline T operator *(const T &p) const {
            return transform(p);
        }
    };
    
    template <typename T>
    struct Translation : public T, public Transform<T> {
        typedef typename T::scalar scalar;
        
        virtual T transform(const T &p) const {
            return p + *this;
        }
        explicit Translation(scalar x = 0, scalar y = 0) : T(x, y){}
        Translation(const T &t) : T(t){}
    };
    
    template <typename T>
    struct AffineTransform : public Transform<T> {
        typedef typename T::scalar scalar;
        
        Translation<T> t;
        scalar angle;
        scalar scaleX;
        scalar scaleY;
        
        virtual T transform(const T &p) const;
        
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
        
        virtual point transform(const point &p) const {
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

