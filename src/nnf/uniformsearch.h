/* 
 * File:   uniformsearch.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 11:01 AM
 */

#ifndef UNIFORMSEARCH_H
#define	UNIFORMSEARCH_H

#include "trypatch.h"
#include "../im/patch.h"
#include "../sampling/uniform.h"

namespace pm {
    
    /**
     * Uniform search for NNF improvement
     */
    template < typename Patch = Patch2ti, typename DistValue = float >
    class UniformSearch {
    public:
        typedef NearestNeighborField<Patch, DistValue> NNF;

        bool operator()(const Point2i &i, bool);

        UniformSearch(NNF *nnf);
    };
    
    // Implementation for 2d translation patches
    template < typename S, typename DistValue >
    class UniformSearch<BasicPatch<S>, DistValue> {
    public:
        typedef BasicPatch<S> TargetPatch;
        typedef typename BasicPatch<S>::point point;
        typedef typename point::vec vec;
        typedef NearestNeighborField<TargetPatch, DistValue> NNF;

        bool operator()(const Point2i &i, bool){
            
            // maximum
            const FrameSize &target = nnf->targetSize().shrink(TargetPatch::width());
            
            // uniformly sample a position for the new patch
            const point &q = uniform(
                nnf->rng(),
                vec(0, 0),
                vec(target.width, target.height)
            );
            return tryPatch(nnf, i, TargetPatch(q));
        }

        UniformSearch(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
}

#endif	/* UNIFORMSEARCH_H */

