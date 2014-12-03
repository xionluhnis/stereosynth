/* 
 * File:   patch.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
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
    struct BasicPatch : public Translation< Point<S>, Point2i > {
        typedef BasicPatch<int> SourcePatch;
        typedef Point<S> point;
        typedef Translation< Point<S>, Point2i > translation;
        
        static int width(int newSize = 0);
        
        bool operator==(const BasicPatch<S> &p) const {
            return p.x == this->x && p.y == this->y;
        }
        
        BasicPatch(const translation &t) : translation(t){}
        BasicPatch(const point &p) : translation(p){}
        BasicPatch() : translation() {}
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
    struct AffinePatch : public AffineTransform< Point<S> > {
        typedef BasicPatch<int> SourcePatch;
        typedef Point<S> point;
        typedef Translation<point> translation;
        typedef AffineTransform<point> affine;
        
        inline static int width(int newSize = 0) {
            return SourcePatch::width(newSize);
        }
        
        bool operator==(const AffinePatch<S> &p) const {
            return p.t.x == this->t.x && p.t.y == this->t.y
                && p.angle == this->angle && p.scaleX == this->scaleX && p.scaleY == this->scaleY;
        }
        
        AffinePatch(const affine &aff) : affine(aff){}
        AffinePatch() : affine(){}
    };
    
    // type names
    typedef AffinePatch<float> Patch2af;
    typedef AffinePatch<double> Patch2ad;
    
    ////////////////////////////////////////////////////////////////////////////
    ///// Patch Pixel Frame ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    template < typename Patch = BasicPatch<int> >
    inline Frame2D< Point<int> > pixels() {
        return Frame2D< Point<int> >(Patch::width(), Patch::width());
    }
	
	template < typename Patch = BasicPatch<int> >
	inline Frame2D< Point<int> > pixels(const Patch &) {
		return pixels< Patch >();
	}
}

#endif	/* IM_PATCH_H */

