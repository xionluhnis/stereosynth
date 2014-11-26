/* 
 * File:   defs.h
 * Author: akaspar
 *
 * Created on November 25, 2014, 3:23 PM
 */

#ifndef VOTING_DEFS_H
#define	VOTING_DEFS_H

#include "../im/patch.h"
#include "../math/mat.h"

#include <iostream>

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
        Image compute();
    };
    
    template <int channels>
    void compute(const VoteOperation &vote, int numChannels) {
        if(channels == numChannels){
            return vote.compute<channels>();
        } else {
            return compute<channels + 1>(vote, numChannels);
        }
    }
    
#ifndef MAX_SUPPORTED_CHANNELS
#define MAX_SUPPORTED_CHANNELS 12
#endif
    
    template<>
    Image compute<MAX_SUPPORTED_CHANNELS+1>(const VoteOperation &vote, int numChannels) {
        std::cerr << "We support only up to " << MAX_SUPPORTED_CHANNELS << "\n";
        return Image();
    }
    
}

#endif	/* VOTING_DEFS_H */

