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
        virtual T transform(const T &p) const {
            return p + *this;
        }
    };
    
    template <typename T>
    struct AffineTransform : public Transform<T> {
        
        typedef typename T::scalar scalar;
        
        Translation<T> t;
        scalar angle;
        scalar scaleX;
        scalar scaleY;
        
        virtual T transform(const T &p) const;
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

