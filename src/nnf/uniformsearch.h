/* 
 * File:   uniformsearch.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
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
    template < typename Patch = Patch2ti, typename DistValue = float, int K = 7 >
    class UniformSearch {
    public:
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        bool operator()(const Point2i &i, bool);

        UniformSearch(NNF *nnf);
    };
    
    // Implementation for 2d translation patches
    template < typename S, typename DistValue>
    class UniformSearch<BasicPatch<S>, DistValue, 1> {
    public:
        typedef BasicPatch<S> TargetPatch;
        typedef typename BasicPatch<S>::point point;
        typedef typename point::vec vec;
        typedef NearestNeighborField<TargetPatch, DistValue, 1> NNF;

        bool operator()(const Point2i &i, bool){
            
            // maximum
            const FrameSize &target = nnf->targetSize().shrink(TargetPatch::width());
            
            // uniformly sample a position for the new patch
            const point &q = uniform(
                nnf->rng(),
                vec(0, 0),
                vec(target.width, target.height)
            );
            return tryPatch<TargetPatch, DistValue>(nnf, i, TargetPatch(q));
        }

        UniformSearch(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
    // Implementation for 2d translation patches and k-NN
    template < typename S, typename DistValue, int K>
    class UniformSearch<BasicPatch<S>, DistValue, K> {
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
                const point &q = uniform(
                    nnf->rng(),
                    vec(0, 0),
                    vec(target.width, target.height)
                );
                success |= kTryPatch<K, TargetPatch, DistValue>(nnf, i, TargetPatch(q));
            }
            return success;
        }

        UniformSearch(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
}

#endif	/* UNIFORMSEARCH_H */

