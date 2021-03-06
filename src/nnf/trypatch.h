/* 
 * File:   trypatch.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 21, 2014, 10:56 AM
 */

#ifndef TRYPATCH_H
#define	TRYPATCH_H

#include "nnf.h"

namespace pm {
    
    template <typename TargetPatch = Patch2ti, typename DistValue = float>
    uint tryPatch(NearestNeighborField<TargetPatch, DistValue, 1> *nnf, const Point2i &i, const TargetPatch &q) {
        // filter patch based on location
        if(nnf->filter(i, q))
            return 0;
        // check whether it's the same
        TargetPatch &p = nnf->patch(i);
        // if it's the same patch, too bad
        if(p == q){
            return 0;
        }
        // compute distance for the new patch
        DistValue newDist = nnf->dist(i, q);
        DistValue &curDist = nnf->distance(i);

        if(newDist < curDist){
            p = q; // replace patch
            curDist = newDist; // store new distance
            return 1;
        }
        return 0;
    }

    template <int K = 7, typename TargetPatch = Patch2ti, typename DistValue = float>
    uint kTryPatch(NearestNeighborField<TargetPatch, DistValue, K> *nnf, const Point2i &i, const TargetPatch &q) {
        // filter patch based on location
        if(nnf->filter(i, q))
            return 0;
        // check whether the patch is already present on the heap
        for(int k = 0; k < K; ++k){
            if(nnf->patch(i, k) == q
            && nnf->distance(i, k) < std::numeric_limits<DistValue>::max()){
                return 0;
            }
        }
        // compute distance for the new patch
        DistValue newDist = nnf->dist(i, q);
        const DistValue &curDist = nnf->distance(i, 0); // worst distance

        if(newDist < curDist){
            return nnf->store(i, q, newDist) ? 1 : 0;
        }
        return 0;
    }
}

#endif	/* TRYPATCH_H */

