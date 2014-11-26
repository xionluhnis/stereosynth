/* 
 * File:   int_nnf_container.h
 * Author: akaspar
 *
 * Created on November 26, 2014, 2:36 PM
 */

#ifndef INT_NNF_CONTAINER_H
#define	INT_NNF_CONTAINER_H

#include "int_single_nnf.h" // we base ourself on 1-NNF since this is for voting
#include "../im/patch.h"
#include "../math/bounds.h"
#include "../nnf/nnf.h"
#include "../voting/defs.h"

namespace pm {
    
    template < int numChannels >
    struct PixelContainer<numChannels, Patch2ti, float> {
        typedef Vec<float, numChannels> vec;
        typedef Point2i point;
        typedef NearestNeighborField<Patch2ti, float, 1> NNF;

        enum {
            channels = numChannels
        };

        Frame2D<Point2i, true> frame() const {
            return Frame2D<Point2i, true>(nnf->targetSize());
        }
        const vec &pixel(const point &p) const {
            return nnf->target.at<vec>(p);
        }
        SubFrame2D<Point2i, true> overlap(const point &p) const {
            Bounds2i frame(Vec2i(0, 0), Vec2i(nnf->source.width, nnf->source.height));
            Bounds2i zone = frame & Bounds2i(p - Vec2i(Patch2ti::width() - 1, Patch2ti::width() - 1), p);
            return SubFrame2D<Point2i, true>(zone.min, zone.max);
        }
        const Patch2ti &patch(const point &i) const {
            return nnf->patches.at(i);
        }

        PixelContainer(NNF *n) : nnf(n) {}
    private:
        NNF *nnf;
    };
    
}

#endif	/* INT_NNF_CONTAINER_H */

