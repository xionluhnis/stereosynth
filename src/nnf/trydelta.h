/* 
 * File:   trydelta.h
 * Author: akaspar
 *
 * Created on December 4, 2014, 11:13 AM
 */

#ifndef TRYDELTA_H
#define	TRYDELTA_H

#include "trypatch.h"
#include "../im/patch.h"

namespace pm {
    
    template <typename TargetPatch = Patch2ti, typename DistValue = float>
    bool tryDelta(NearestNeighborField<TargetPatch, DistValue, 1> *nnf, const Point2i &i, const Point2i &delta) {
        Point2i j = i - delta;
        if(nnf->contains(j)){
            TargetPatch q = nnf->patch(j).transform(delta); // transform delta in patch => new position => new patch
            if(!isValid(nnf, q))
                return false;
            return tryPatch(nnf, i, q);
        }
        return false;
    }

    template <int K = 7, typename TargetPatch = Patch2ti, typename DistValue = float>
    bool kTryDelta(NearestNeighborField<TargetPatch, DistValue, K> *nnf, const Point2i &i, const Point2i &delta) {
        bool success = false;
        Point2i j = i - delta;
        if(nnf->contains(j)){
            TargetPatch q[K];
            for(int k = 0; k < K; ++k){
                q[k] = nnf->patch(j, k).transform(delta); // transform delta in patch => new position => new patch
            }
            for(int k = 0; k < K; ++k){
                // check that the patch is valid in the target
                if(!isValid(nnf, q[k]))
                    continue;
                success |= kTryPatch<K, TargetPatch, DistValue>(nnf, i, q[k]);
            }
        }
        return success;
    }
}

#endif	/* TRYDELTA_H */

