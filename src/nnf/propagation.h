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
    
    template < typename NNF, typename Patch = typename NNF::TargetPatch >
    class Propagation : public TryPatch<NNF, Patch> {
    public:

        bool tryDelta(const Point2i &i, const Point2i &delta) {
            Point2i j = i - delta;
            if(nnf->contains(j)){
                TargetPatch q = nnf->patches.at(j).transform(delta); // transform delta in patch => new position => new patch
                return tryPatch(i, q);
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

        Propagation(NNF *nnf) : TryPatch(nnf){}
    };
    
}

#endif	/* PROPAGATION_H */

