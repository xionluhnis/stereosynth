/* 
 * File:   localmean.h
 * Author: akaspar
 *
 * Created on December 4, 2014, 6:43 PM
 */

#ifndef LOCALMEAN_H
#define	LOCALMEAN_H

#include "field.h"
#include "trypatch.h"
#include "../im/patch.h"
#include "../sampling/uniform.h"
#include "nnf.h"

namespace pm {
    
    inline Point2i sampleNeighbor(const FrameSize &frame, RNG rng, const Point2i &center, int radius){
        Bounds2i rect(Vec2i(0, 0), Vec2i(frame.width - 1, frame.height - 1));
        Bounds2i bounds = rect & Bounds2i(center, radius);
        uint i = 0;
        do {
            // TODO compute it in 1D and map into 2D since it's discrete (with exclusion of center => direct result, no loop)
            const Point2i &v = uniform<Vec2i>(rng, bounds.min, bounds.max);
            if(v != center){
                return v;
            }
        }while(++i < 10);
        return center;
    }
    
    template < typename Patch = Patch2ti, typename DistValue = float, int K = 7, int N = 4>
    class LocalMean {
    public:
        typedef NearestNeighborField<Patch, DistValue, K> NNF;
        typedef typename Patch::point point;

        uint operator()(const Point2i &i, bool rev) {
            Point2f q(0, 0);
            for(int j = 0; j < N; ++j){
                int k = uniform<int>(nnf->rng(), 0, K-1);
                const Point2i &n = sampleNeighbor(nnf->frameSize(), nnf->rng(), i, radius);
                q = q + Point2f(nnf->patch(n, k));
            }
            point p(q * (1.0f / N));
            return kTryPatch(nnf, i, Patch(p)); // propagate from randomly far point
        }

        LocalMean(NNF *n, int r = 1) : nnf(n), radius(r){
            assert(radius >= 0 && "Radius should be positive");
        }
        
    private:
        NNF *nnf;
        int radius;
    };
    
    template < typename Patch, typename DistValue, int N>
    class LocalMean<Patch, DistValue, 1, N> {
    public:
        typedef NearestNeighborField<Patch, DistValue, 1> NNF;
        typedef typename Patch::point point;

        uint operator()(const Point2i &i, bool rev) {
            Point2f q(0, 0);
            for(int j = 0; j < N; ++j){
                const Point2i &n = sampleNeighbor(nnf->frameSize(), nnf->rng(), i, radius);
                q = q + Point2f(nnf->patch(n));
            }
            point p(q * (1.0f / N));
            return tryPatch(nnf, i, Patch(p)); // propagate from randomly far point
        }

        LocalMean(NNF *n, int r) : nnf(n), radius(r){
            assert(radius >= 0 && "Radius should be positive");
        }
        
    private:
        NNF *nnf;
        int radius;
    };
    
}

#endif	/* LOCALMEAN_H */

