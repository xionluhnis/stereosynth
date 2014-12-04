/* 
 * File:   horizontalrandsearch.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 2, 2014, 9:41 AM
 */

#ifndef HORIZONTALRANDSEARCH_H
#define	HORIZONTALRANDSEARCH_H

#include "nnf.h"
#include "searchradius.h"
#include "trypatch.h"
#include "../im/patch.h"
#include "../math/bounds.h"

namespace pm {
    
    template < typename Patch = Patch2ti, typename DistValue = float, int K = 7 >
    struct HorizontalRandomSearch {
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        uint operator()(const Point2i &i, bool);

        HorizontalRandomSearch(NNF *nnf);
    };
    
    // Implementation for 2d translation patches
    template < typename S, typename DistValue>
    class HorizontalRandomSearch<BasicPatch<S>, DistValue, 1> {
    public:
        typedef BasicPatch<S> TargetPatch;
        typedef typename BasicPatch<S>::point point;
        typedef typename point::vec vec;
        typedef NearestNeighborField<TargetPatch, DistValue, 1> NNF;
        typedef Bounds<S, 2> bounds;

        uint operator()(const Point2i &i, bool){
            
            // target patch
            const TargetPatch &p = nnf->patch(i);
            
            // search bounds
            const FrameSize &target = nnf->targetSize().shrink(TargetPatch::width());
            bounds b = bounds(
                vec(S(0), std::max<S>(0, i.y - maxDY)),
                vec(S(target.width), std::min<S>(target.height, i.y + maxDY))
            ) & bounds(p, search->radius);
            
            // uniformly sample a position for the new patch
            const point &q = uniform<vec>(nnf->rng(), b.min, b.max);
            return tryPatch<TargetPatch, DistValue>(nnf, i, TargetPatch(q));
        }

        HorizontalRandomSearch(NNF *n, const SearchRadius<S> *sr, S dy = 5) : nnf(n), search(sr), maxDY(dy){}
        
    private:
        NNF *nnf;
        const SearchRadius<S> *search;
        S maxDY;
    };
    
    // Implementation for 2d translation patches and k-NN
    template < typename S, typename DistValue, int K>
    class HorizontalRandomSearch<BasicPatch<S>, DistValue, K> {
    public:
        typedef BasicPatch<S> TargetPatch;
        typedef typename BasicPatch<S>::point point;
        typedef typename point::vec vec;
        typedef NearestNeighborField<TargetPatch, DistValue, K> NNF;
        typedef Bounds<S, 2> bounds;

        uint operator()(const Point2i &i, bool){
            
            // target patches
            point p[K];
            for(int k = 0; k < K; ++k){
                p[k] = nnf->patch(i, k);
            }
            
            // bounds
            const FrameSize &target = nnf->targetSize().shrink(TargetPatch::width());
            bounds frame(
                vec(S(0), std::max<S>(0, i.y - maxDY)),
                vec(S(target.width), std::min<S>(target.height, i.y + maxDY))
            );
            
            // sample in window defined by the current patch and the given radius
            uint success = 0;
            for(int k = 0; k < K; ++k){
                bounds b = frame & bounds(p[k], search->radius);
                const point &q = uniform<vec>(nnf->rng(), b.min, b.max);
                success += kTryPatch<K, TargetPatch, DistValue>(nnf, i, TargetPatch(q));
            }
            return success;
        }

        HorizontalRandomSearch(NNF *n, const SearchRadius<S> *sr, S dy) : nnf(n), search(sr), maxDY(dy){}
        
    private:
        NNF *nnf;
        const SearchRadius<S> *search;
        S maxDY;
    };
    
}

#endif	/* HORIZONTALRANDSEARCH_H */

