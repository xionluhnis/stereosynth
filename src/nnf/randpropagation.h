/* 
 * File:   randpropagation.h
 * Author: akaspar
 *
 * Created on December 4, 2014, 11:10 AM
 */

#ifndef RANDPROPAGATION_H
#define	RANDPROPAGATION_H

#include "field.h"
#include "trydelta.h"
#include "../im/patch.h"
#include "../sampling/uniform.h"
#include "nnf.h"

namespace pm {
    
    template < bool RowMajor = true >
    Point2i samplePoint(const Field2D<RowMajor> *field, RNG rng){
        const FrameSize &frame = field->frameSize();
        return uniform<Vec2i>(
            rng,
            Vec2i(0, 0),
            Vec2i(frame.width, frame.height)
        );
    }
    
    template < typename Patch = Patch2ti, typename DistValue = float, int K = 7>
    class RandomPropagation {
    public:
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        bool operator()(const Point2i &i, bool rev) {
            const Point2i &q = samplePoint(nnf, nnf->rng());
            return kTryDelta(nnf, i, i - q); // propagate from randomly far point
        }

        RandomPropagation(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
    template < typename Patch, typename DistValue>
    class RandomPropagation<Patch, DistValue, 1> {
    public:
        typedef NearestNeighborField<Patch, DistValue, 1> NNF;

        bool operator()(const Point2i &i, bool rev) {
            const Point2i &q = samplePoint(nnf, nnf->rng());
            return tryDelta(nnf, i, i - q); // propagate from randomly far point
        }

        RandomPropagation(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
}

#endif	/* RANDPROPAGATION_H */

