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
    };
    
    template<typename S>
    Point<S> AffineTransform< Point<S> >::transform(const Point<S> &p) const {
        Point<S> q = p;
        q.x *= scaleX;
        q.y *= scaleY;
        
        scalar cosA = std::cos(angle);
        scalar sinA = std::sin(angle);
        
        scalar tx = cosA * px - sinA * py;
        scalar ty = sinA * px + cosA * py;
        return Point<S>(tx, ty) + t;
    }
    
}

#endif	/* TRANSFORM_H */

