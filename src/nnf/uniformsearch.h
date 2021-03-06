/* 
 * File:   uniformsearch.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 21, 2014, 11:01 AM
 */

#ifndef UNIFORMSEARCH_H
#define	UNIFORMSEARCH_H

#include "trypatch.h"
#include "../nnf/patch.h"
#include "../sampling/uniform.h"

namespace pm {
    
    /**
     * Uniform search for NNF improvement
     */
    template < typename Patch = Patch2ti, typename DistValue = float, int K = 7 >
    class UniformSearch {
    public:
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        uint operator()(const Point2i &i, bool);

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

        uint operator()(const Point2i &i, bool){
            
            // maximum
            const FrameSize target = nnf->targetSize().shrink(TargetPatch::width());
            
            // uniformly sample a position for the new patch
            const point q = uniform(
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

        uint operator()(const Point2i &i, bool){
            
            // maximum
            const FrameSize target = nnf->targetSize().shrink(TargetPatch::width());
            
            // uniformly sample a position for the new patch
            uint success = 0;
            for(int k = 0; k < K; ++k){
                const point q = uniform(
                    nnf->rng(),
                    vec(0, 0),
                    vec(target.width, target.height)
                );
                success += kTryPatch<K, TargetPatch, DistValue>(nnf, i, TargetPatch(q));
            }
            return success;
        }

        UniformSearch(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
    // Implementation for 2d+1x translation patches and k-NN
    template < typename S, typename DistValue, int K>
    class UniformSearch<BasicIndexedPatch<S>, DistValue, K> {
    public:
        typedef BasicIndexedPatch<S> TargetPatch;
        typedef typename BasicIndexedPatch<S>::point point;
        typedef typename point::base base;
        typedef typename point::vec vec3; // this is not Vec2x !!!
        typedef Vec<S, 2> vec2; // real vector to use for points
        typedef NearestNeighborField<TargetPatch, DistValue, K> NNF;

        uint operator()(const Point2i &i, bool){
            // uniformly sample a position for the new patch
            uint success = 0;
            for(int k = 0; k < K; ++k){
                // uniformly sample image from set
                int idx = uniform<int>(nnf->rng(), 0, nnf->targetCount() - 1);
                assert(idx < nnf->targetCount() && "Invalid target index");
                // frame bounds
                const FrameSize target = nnf->targetSize(idx).shrink(TargetPatch::width());
                // uniformly sample a position within that target image
                const base q = uniform(
                    nnf->rng(),
                    vec2(0, 0), // not vec3 !
                    vec2(target.width, target.height)
                );
                success += kTryPatch<K, TargetPatch, DistValue>(nnf, i, TargetPatch(q, idx));
            }
            return success;
        }

        UniformSearch(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
}

#endif	/* UNIFORMSEARCH_H */

