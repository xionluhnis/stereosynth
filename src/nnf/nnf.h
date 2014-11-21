/* 
 * File:   nnf.h
 * Author: akaspar
 *
 * Created on November 21, 2014, 11:17 AM
 */

#ifndef NNF_H
#define	NNF_H

#include "distance.h"
#include "field.h"
#include "../im/patch.h"
#include "../sampling/uniform.h"

namespace pm {
    
    // distance type
    typedef Distance<Patch2ti, float> DistanceFunc;

    // nearest neighbor field
    template < typename Patch = Patch2ti, typename DistValue = float >
    struct NearestNeighborField : Field2D<true> {

        typedef Patch TargetPatch;
        typedef DistValue DistanceValue;
        typedef typename Patch::point point;
        typedef typename point::vec vec;
        
        // --- provide a common RNG instance -----------------------------------
        RNG rng() const;
        // --- provide a target dimension --------------------------------------
        FrameSize targetSize() const;
        // --- provide patch and distance storage ------------------------------
        Entry<TargetPatch> patches;
        Entry<DistValue> distances;
        // --- provide a distance computation ----------------------------------
        DistanceValue dist(const Point2i &pos, const TargetPatch &q);

    };
    
}

#endif	/* NNF_H */

