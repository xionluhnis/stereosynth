/* 
 * File:   patch.h
 * Author: akaspar
 *
 * Created on November 17, 2014, 7:06 PM
 */

#ifndef IM_PATCH_H
#define	IM_PATCH_H

#ifndef DEFAULT_PATCH_SIZE
#define DEFAULT_PATCH_SIZE 7
#endif

#include "../math/iterator2d.h"
#include "../math/point.h"
#include "../math/transform.h"

namespace pm {
    
    ////////////////////////////////////////////////////////////////////////////
    ///// Basic Translation Patch //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    template < typename S >
    struct BasicPatch : public Translation< Point<S> >, public Iterable2D< Point<S>, true > {
        typedef BasicPatch<int> SourcePatch;
        
        static int width(int newSize = 0);
        
        virtual int size0() const {
            return width();
        }
        virtual int size1() const {
            return width();
        }
    };
    template <>
    inline int BasicPatch<int>::width(int newSize) {
        static int size = DEFAULT_PATCH_SIZE;
        if(newSize > 0){
            size = newSize;
        }
        return size;
    }
    template <>
    inline int BasicPatch<float>::width(int newSize) {
        return SourcePatch::width(newSize); // delegate to SourcePatch type
    }
    
    // type names
    typedef BasicPatch<int> Patch2ti;
    typedef BasicPatch<float> Patch2tf;
    typedef BasicPatch<double> Patch2td;
    
    ////////////////////////////////////////////////////////////////////////////
    ///// Affine Patch /////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    template < typename S >
    struct AffinePatch : public AffineTransform< Point<S> >, public Iterable2D< Point<S>, true > {
        typedef BasicPatch<int> SourcePatch;
        
        inline static int width(int newSize = 0) {
            return SourcePatch::width(newSize);
        }
        
        virtual int size0() const {
            return width();
        }
        virtual int size1() const {
            return width();
        }
    };
    
    // type names
    typedef AffinePatch<float> Patch2af;
    typedef AffinePatch<double> Patch2ad;
}

#endif	/* IM_PATCH_H */

