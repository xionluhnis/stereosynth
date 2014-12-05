/* 
 * File:   defs.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 25, 2014, 3:23 PM
 */

#ifndef VOTING_DEFS_H
#define	VOTING_DEFS_H

#include "../im/patch.h"
#include "../math/mat.h"

#include <iostream>

namespace pm {
    
    template < int numChannels, typename Patch = Patch2ti, typename Scalar = float, int K = 1 >
    struct PixelContainer {
        typedef Scalar scalar;
        typedef Vec<Scalar, numChannels> vec;
        typedef typename Patch::point PixelLoc;
        
        enum {
            channels = numChannels
        };
        
        Frame2D<Point2i, true> frame() const;
        vec pixel(const PixelLoc &p) const;
        SubFrame2D<Point2i, true> overlap(const Point2i &p) const;
        const Patch &patch(const Point2i &i, int k = 0) const;
        const Scalar &distance(const Point2i &i, int k = 0) const;
    };
    
    template <int channels, typename VoteOperation>
    struct Vote {
        typedef typename VoteOperation::Next NextOperation;
        
        Image compute(int numChannels) {
            if(channels == numChannels){
                return vote.compute();
            } else {
                return Vote<channels + 1, NextOperation>(vote).compute(numChannels);
            }
        }
        
        Vote(const VoteOperation &v) : vote(v) {}
        
    private:
        VoteOperation vote;
    };
    
#ifndef MAX_SUPPORTED_CHANNELS
#define MAX_SUPPORTED_CHANNELS 12
#endif
    
    template <typename VoteOperation>
    struct Vote<MAX_SUPPORTED_CHANNELS+1, VoteOperation> {
        Image compute(int numChannels) {
            std::cerr << "We support only up to " << MAX_SUPPORTED_CHANNELS << " (curr=" << numChannels << ")\n";
            return Image();
        }
        
        Vote(const VoteOperation &v) {}
    };
    
    template <typename VoteOperation>
    Image vote(const VoteOperation &op, int numChannels){
        Vote<1, VoteOperation> v(op);
        return v.compute(numChannels);
    }
    
}

#endif	/* VOTING_DEFS_H */

