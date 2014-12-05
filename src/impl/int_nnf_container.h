/* 
 * File:   int_nnf_container.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 26, 2014, 2:36 PM
 */

#ifndef INT_NNF_CONTAINER_H
#define	INT_NNF_CONTAINER_H

#include "int_single_nnf.h" // we base ourself on 1-NNF since this is for voting
#include "../nnf/patch.h"
#include "../math/bounds.h"
#include "../nnf/nnf.h"
#include "../voting/defs.h"

namespace pm {
    
    template < int numChannels >
    struct PixelContainer<numChannels, Patch2ti, float, 1> {
        typedef Vec<float, numChannels> vec;
        typedef Point2i PixelLoc;
        typedef NearestNeighborField<Patch2ti, float, 1> NNF;

        enum {
            channels = numChannels
        };

        Frame2D<Point2i, true> frame() const {
            return Frame2D<Point2i, true>(FrameSize(nnf->source.width, nnf->source.height));
        }
        const vec &pixel(const PixelLoc &p) const {
#ifdef DEBUG_STRICT_TEST
            assert(nnf->target.contains(p) && "Patch out of bounds");
#endif			
            return nnf->target.at<vec>(p);
        }
        SubFrame2D<Point2i, true> overlap(const Point2i &p) const {
#ifdef DEBUG_STRICT_TEST
            assert(nnf->source.contains(p) && "Patch out of bounds");
#endif
            Bounds2i frame(Vec2i(0, 0), Vec2i(nnf->width, nnf->height));
            Bounds2i zone = frame & Bounds2i(p - Vec2i(Patch2ti::width() - 1, Patch2ti::width() - 1), p + Vec2i(1, 1));
            return SubFrame2D<Point2i, true>(zone.min, zone.max);
        }
        const Patch2ti &patch(const Point2i &i, int = 1) const {
#ifdef DEBUG_STRICT_TEST
            assert(nnf->contains(i) && "Patch out of bounds");
#endif
            return nnf->patch(i);
        }
        
        const float &distance(const Point2i &i, int = 1) const {
#ifdef DEBUG_STRICT_TEST
            assert(nnf->contains(i) && "Patch distance out of bounds");
#endif
            return nnf->distance(i);
        }

        PixelContainer(NNF *n) : nnf(n) {}
    private:
        NNF *nnf;
    };
    
}

#endif	/* INT_NNF_CONTAINER_H */

