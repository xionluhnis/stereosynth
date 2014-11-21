/* 
 * File:   trypatch.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 10:56 AM
 */

#ifndef TRYPATCH_H
#define	TRYPATCH_H

#include "field.h"

namespace pm {
    
    /**
     * Base implementation for patch algorithm
     */
    template < typename NNF, typename Patch = typename NNF::TargetPatch >
    class TryPatch {
    public:
        
        typedef typename Patch TargetPatch;
        typedef typename TargetPatch::SourcePatch SourcePatch;
        typedef typename NNF::DistanceType DistanceValue;
        typedef TryPatch<NNF, Patch> Base;

        bool tryPatch(const Point2i &i, const TargetPatch &q) {
            TargetPatch &p = nnf->patches.at(i);
            // if it's the same patch, too bad
            if(p == q){
                return false;
            }
            // compute distance for the new patch
            DistanceValue newDist = nnf->dist(i, q);
            DistanceValue &curDist = nnf->distances.at(i);

            if(newDist < curDist){
                p = q; // replace patch
                curDist = newDist; // store new distance
                return true;
            }
            return false;
        }

        TryPatch(NNF *n) : nnf(n){}
    protected:
        NNF *nnf;
    };
    
}

#endif	/* TRYPATCH_H */

