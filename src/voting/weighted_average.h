/* 
 * File:   weighted_average.h
 * Author: akaspar
 *
 * Created on November 25, 2014, 3:17 PM
 */

#ifndef WEIGHTED_AVERAGE_H
#define	WEIGHTED_AVERAGE_H

#include "defs.h"
#include "../math/filter.h"
#include "../math/mat.h"

namespace pm {
    
    /**
	 * \brief Vote by using a simple filter over overlapping patches
	 * 
	 * \param nnf
	 *			the nearest neighbor field to vote with
	 * \param params
	 *			the voting parameters
	 * \return the voted picture
	 */
	template <int channels, typename Patch, typename Scalar>
	Image weighted_average(const PixelContainer<channels, Patch, Scalar> &data, const Filter &filter) {
		typedef Vec<Scalar, channels> Pixel;
        
        // create workspace
        const Frame2D<Point2i, true> &frame = data.frame();
		Image vote = Image::zeros(frame.size.rows, frame.size.cols, IM_32FC(channels));

		// for each pixel of the workspace
        for(const Point2i &i : frame){
            Pixel &votedPixel = vote.at<Pixel>(i);
            Scalar weight = 0;
            // use weighted average of overlapping patch pixels
            for(const Point2i &p : data.overlap(i)){
                const Patch &patch = data.patch(p);
                const Point2i b = i - p;
                const Pixel &pixel = data.pixel(patch.transform(b));
                votedPixel += pixel * filter[b.y][b.x];
                weight += filter[b.y][b.x];
            }
            if(weight > 1e-8){
                votedPixel *= 1.0 / weight;
            }
        }
		return vote;
	}
    
}

#endif	/* WEIGHTED_AVERAGE_H */

