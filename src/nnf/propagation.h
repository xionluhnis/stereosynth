/* 
 * File:   propagation.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 21, 2014, 11:13 AM
 */

#ifndef PROPAGATION_H
#define	PROPAGATION_H

#include "trypatch.h"
#include "trydelta.h"
#include "../nnf/patch.h"

namespace pm {
    
    template < typename Patch = Patch2ti, typename DistValue = float, int K = 7>
    class Propagation {
    public:
        typedef NearestNeighborField<Patch, DistValue, K> NNF;

        uint operator()(const Point2i &i, bool rev) {
            // direction for deltas
            int d = rev ? -1 : 1;
            // two propagation tentatives
            uint res = 0;
            res += kTryDelta(nnf, i, Point2i(d, 0)); // dx
            res += kTryDelta(nnf, i, Point2i(0, d)); // dy
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

        uint operator()(const Point2i &i, bool rev) {
            // direction for deltas
            int d = rev ? -1 : 1;
            // two propagation tentatives
            uint res = 0;
            res += tryDelta(nnf, i, Point2i(d, 0)); // dx
            res += tryDelta(nnf, i, Point2i(0, d)); // dy
            return res;
        }

        Propagation(NNF *n) : nnf(n){}
        
    private:
        NNF *nnf;
    };
    
}

#endif	/* PROPAGATION_H */

