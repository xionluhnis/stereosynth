/* 
 * File:   horizontalsearch.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 2, 2014, 9:31 AM
 */

#ifndef HORIZONTALSEARCH_H
#define	HORIZONTALSEARCH_H

#include "trypatch.h"
#include "../im/patch.h"
#include "../sampling/uniform.h"

namespace pm {
    
    /**
     * Uniform horizontal search for NNF improvement
     */
    template < typename Patch = Patch2ti, typename DistValue = float, int K = 7 >
    class HorizontalSearch {
    public:
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        bool operator()(const Point2i &i, bool);

        HorizontalSearch(NNF *nnf);
    };
    
    // Implementation for 2d translation patches
    template < typename S, typename DistValue>
    class HorizontalSearch<BasicPatch<S>, DistValue, 1> {
    public:
        typedef BasicPatch<S> TargetPatch;
        typedef typename BasicPatch<S>::point point;
        typedef typename point::vec vec;
        typedef NearestNeighborField<TargetPatch, DistValue, 1> NNF;

        bool operator()(const Point2i &i, bool){
            
            // maximum
            const FrameSize &target = nnf->targetSize().shrink(TargetPatch::width());
            
            // uniformly sample a position for the new patch
            point q(uniform<S>(nnf->rng(), 0, target.width), i.y);
            return tryPatch<TargetPatch, DistValue>(nnf, i, TargetPatch(q));
        }

        HorizontalSearch(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
    // Implementation for 2d translation patches and k-NN
    template < typename S, typename DistValue, int K>
    class HorizontalSearch<BasicPatch<S>, DistValue, K> {
    public:
        typedef BasicPatch<S> TargetPatch;
        typedef typename BasicPatch<S>::point point;
        typedef typename point::vec vec;
        typedef NearestNeighborField<TargetPatch, DistValue, K> NNF;

        bool operator()(const Point2i &i, bool){
            
            // maximum
            const FrameSize &target = nnf->targetSize().shrink(TargetPatch::width());
            
            // uniformly sample a position for the new patch
            bool success = false;
            for(int k = 0; k < K; ++k){
                point q(uniform<S>(nnf->rng(), 0, target.width), i.y);
                success |= kTryPatch<K, TargetPatch, DistValue>(nnf, i, TargetPatch(q));
            }
            return success;
        }

        HorizontalSearch(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
}

#endif	/* HORIZONTALSEARCH_H */

