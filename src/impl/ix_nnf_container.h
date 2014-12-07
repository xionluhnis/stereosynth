/* 
 * File:   ix_nnf_container.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 7, 2014, 5:10 PM
 */

#ifndef IX_NNF_CONTAINER_H
#define	IX_NNF_CONTAINER_H

#include "ix_k_nnf.h" // we base ourself on 1-NNF since this is for voting
#include "../nnf/patch.h"
#include "../math/bounds.h"
#include "../nnf/nnf.h"
#include "../voting/defs.h"

namespace pm {

    template < int numChannels, typename S, int K>
    struct PixelContainer<numChannels, BasicIndexedPatch<S>, float, K> {
        typedef Vec<float, numChannels> vec;
        typedef BasicIndexedPatch<S> TargetPatch;
        typedef typename TargetPatch::point PixelLoc;
        typedef NearestNeighborField<TargetPatch, float, K> NNF;
        
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
            return nnf->targets.template at<vec>(p);
        }
        SubFrame2D<Point2i, true> overlap(const Point2i &p) const {
#ifdef DEBUG_STRICT_TEST
            assert(nnf->source.contains(p) && "Patch out of bounds");
#endif
            Bounds2i frame(Vec2i(0, 0), Vec2i(nnf->width, nnf->height));
            Bounds2i zone = frame & Bounds2i(p - Vec2i(TargetPatch::width() - 1, TargetPatch::width() - 1), p + Vec2i(1, 1));
            return SubFrame2D<Point2i, true>(zone.min, zone.max);
        }
        const TargetPatch &patch(const Point2i &i, int k = 0) const {
#ifdef DEBUG_STRICT_TEST
            assert(nnf->contains(i) && "Patch out of bounds");
#endif
            return nnf->patch(i, k);
        }
        
        const float &distance(const Point2i &i, int k = 0) const {
#ifdef DEBUG_STRICT_TEST
            assert(nnf->contains(i) && "Patch distance out of bounds");
#endif
            return nnf->distance(i, k);
        }

        PixelContainer(NNF *n) : nnf(n) {}
        
    private:
        NNF *nnf;
    };
    
}

#endif	/* IX_NNF_CONTAINER_H */

