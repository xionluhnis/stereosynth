/* 
 * File:   defs.h
 * Author: akaspar
 *
 * Created on November 25, 2014, 3:23 PM
 */

#ifndef VOTING_DEFS_H
#define	VOTING_DEFS_H

#include "../im/patch.h"
#include "../math/iterator2d.h"

namespace pm {
    
    template < int numChannels, typename Patch = Patch2ti, typename Scalar = float >
    struct PixelContainer {
        typedef Scalar scalar;
        typedef Vec<Scalar, numChannels> vec;
        typedef typename Patch::point point;
        
        enum {
            channels = numChannels
        };
        
        Frame2D<Point2i, true> frame() const;
        vec pixel(const point &p) const;
        SubFrame2D<Point2i, true> overlap(const point &p) const;
        const Patch &patch(const point &i) const;
    };
    
    struct VoteOperation {
        template <int channels>
        void compute();
    };
    
    template <int channels>
    void compute(const VoteOperation &vote, int numChannels) {
        if(channels == numChannels){
            vote.compute<channels>();
        } else {
            compute<channels + 1>(vote, numChannels);
        }
    }
    
}

#endif	/* VOTING_DEFS_H */

