/* 
 * File:   propagation.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 11:13 AM
 */

#ifndef PROPAGATION_H
#define	PROPAGATION_H

#include "trypatch.h"
#include "../im/patch.h"

namespace pm {
    
    template < typename Patch = Patch2ti, typename DistValue = float, int K = 7>
    class Propagation {
    public:
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        bool tryDelta(const Point2i &i, const Point2i &delta) {
            bool success = false;
            Point2i j = i - delta;
            if(nnf->contains(j)){
                Patch q[K];
                for(int k = 0; k < K; ++k){
                    q[k] = nnf->patch(j, k).transform(delta); // transform delta in patch => new position => new patch
                }
                for(int k = 0; k < K; ++k){
                    // check that the patch is valid in the target
                    if(!isValid(nnf, q[k]))
                        continue;
                    success |= kTryPatch<K, Patch, DistValue>(nnf, i, q[k]);
                }
            }
            return success;
        }

        bool operator()(const Point2i &i, bool rev) {
            // direction for deltas
            int d = rev ? -1 : 1;
            // two propagation tentatives
            bool res = false;
            res |= tryDelta(i, Point2i(d, 0)); // dx
            res |= tryDelta(i, Point2i(0, d)); // dy
            return res;
        }

        Propagation(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
    template < typename Patch, typename DistValue>
    class Propagation<Patch, DistValue, 1> {
    public:
        typedef NearestNeighborField<Patch, DistValue, 1> NNF;

        bool tryDelta(const Point2i &i, const Point2i &delta) {
            Point2i j = i - delta;
            if(nnf->contains(j)){
                Patch q = nnf->patch(j).transform(delta); // transform delta in patch => new position => new patch
                if(!isValid(nnf, q))
                    return false;
                return tryPatch(nnf, i, q);
            }
            return false;
        }

        bool operator()(const Point2i &i, bool rev) {
            // direction for deltas
            int d = rev ? -1 : 1;
            // two propagation tentatives
            bool res = false;
            res |= tryDelta(i, Point2i(d, 0)); // dx
            res |= tryDelta(i, Point2i(0, d)); // dy
            return res;
        }

        Propagation(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
}

#endif	/* PROPAGATION_H */

