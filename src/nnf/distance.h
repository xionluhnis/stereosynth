/* 
 * File:   distance.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 19, 2014, 3:41 PM
 */

#ifndef NNF_DISTANCE_H
#define	NNF_DISTANCE_H

#include "../im/patch.h"
#include "../math/mat.h"

#include <iostream>

namespace pm {
    
    template <typename TargetPatch, typename Scalar, typename Img = Image>
    using Distance = Scalar(*)(const Img &, const Img &, const typename TargetPatch::SourcePatch &, const TargetPatch &);
    
    namespace dist { 
        
        /**
		 * \brief Simple sum of squared differences
		 */
        template <typename TargetPatch, typename Scalar, typename Img, int numChannels>
		Scalar SumSquaredDiff(const Img &source, const Img &target,
				const typename TargetPatch::SourcePatch &p1, const TargetPatch &p2) {
            typedef typename TargetPatch::SourcePatch SourcePatch;
            typedef Vec<Scalar, numChannels> Pixel;
			const int width = SourcePatch::width();
			const Scalar invArea = 1.0 / (width * width);
			Scalar sum = 0;
			
			for (const auto &i : pixels(p1)) {
				Pixel diff = source.template at<Pixel>(p1.transform(i)) - target.template at<Pixel>(p2.transform(i));
				Scalar d = diff.dot(diff);
				sum += d * invArea;
				if (!std::isfinite(sum)) return sum;
			}
			return sum;
		}
        
        /**
         * Distance identifier
         */
        enum DistanceType {
            SSD,
            Unknown
        };
    }
    
    template <typename Patch, typename Scalar, typename Img = Image, int channels = 1>
    struct DistanceFactory {
        static Distance<Patch, Scalar, Img> get(dist::DistanceType type, int numChannels){
            if(numChannels > channels){
                return DistanceFactory<Patch, Scalar, Img, channels + 1>::get(type, numChannels);
            }
            switch(type){
                default:
                   std::cerr << "Invalid distance type " << type << "\n";
                case dist::SSD:
                    return &dist::SumSquaredDiff<Patch, Scalar, Img, channels>;
            }
        }
    };
    
#ifndef MAX_SUPPORTED_CHANNELS
#define MAX_SUPPORTED_CHANNELS 12
#endif
    
    template <typename Patch, typename Scalar, typename Img>
    struct DistanceFactory<Patch, Scalar, Img, MAX_SUPPORTED_CHANNELS+1> {
        static Distance<Patch, Scalar, Img> get(dist::DistanceType, int){
            std::cerr << "We do not supported more than " << MAX_SUPPORTED_CHANNELS << "\n";
            return NULL;
        }
    };
    
}

#endif	/* NNF_DISTANCE_H */

