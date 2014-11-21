/* 
 * File:   trypatch.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 10:56 AM
 */

#ifndef TRYPATCH_H
#define	TRYPATCH_H

#include "nnf.h"
#include "../im/patch.h"

namespace pm {
    
    template <typename TargetPatch = Patch2ti, typename DistValue = float>
    bool tryPatch(NearestNeighborField<TargetPatch, DistValue> *nnf, const Point2i &i, const TargetPatch &q) {
        TargetPatch &p = nnf->patches.at(i);
        // if it's the same patch, too bad
        if(p == q){
            return false;
        }
        // compute distance for the new patch
        DistValue newDist = nnf->dist(i, q);
        DistValue &curDist = nnf->distances.at(i);

        if(newDist < curDist){
            p = q; // replace patch
            curDist = newDist; // store new distance
            return true;
        }
        return false;
    }
    
}

#endif	/* TRYPATCH_H */

