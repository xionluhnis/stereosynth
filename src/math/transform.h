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
    struct AffineTransform : public T, public Transform<T> {
        
    };
    
}

#endif	/* TRANSFORM_H */

