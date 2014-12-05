/* 
 * File:   disparity_vote.h
 * Author: akaspar
 *
 * Created on December 3, 2014, 5:52 PM
 */

#ifndef DISPARITY_VOTE_H
#define	DISPARITY_VOTE_H

#include "defs.h"
#include "../math/filter.h"
#include "../math/mat.h"
#include "../nnf/field.h"

#include <algorithm>
#include <map>
#include <vector>

namespace pm {
    
    namespace voting {
        
        struct CDF {
            
            CDF(std::vector<float> &data, int sections = 100){
                // sort data in ascending order
                std::sort(data.begin(), data.end());
                
                // build cdf map
                prctile.resize(sections + 1);
                cdf[data.front()] = 0.0f;
                cdf[data.back()] = 1.0f;
                size_t N = data.size();
                for(int i = 1; i < sections; ++i){
                    size_t idx = std::round(float(i) / float(sections) * N);
                    float item = data[idx];
                    cdf[item] = float(i) / sections;
                    prctile[i] = item; // store percentile for fast lookup
                }
            }
            
            inline float tail(float value) const {
                auto it = cdf.upper_bound(value);
                if(it == cdf.end()) return 1.0f;
                return it->second;
            }
            
            inline float head(float value) const {
                return 1.0f - tail(value); 
            }
            
            inline float percentile(float value) const {
                size_t n = std::max(size_t(0), std::min(prctile.size() - 1, 
                    size_t(std::floor(prctile.size() * value))
                ));
                return prctile[n];
            }
            
        private:
            std::map<float, float> cdf;
            std::vector<float> prctile;
        };
        
        template <typename Patch >
        inline auto disparity(const Point2i &i, const Patch &q){
            typedef typename Patch::point PixelLoc;
            typedef typename PixelLoc::vec Disparity;
            PixelLoc p(i);
            return Disparity(PixelLoc(q) - p);
        }
        
    }
    
    /**
	 * \brief Vote by using a simple filter over overlapping patches
	 * 
	 * \param nnf
	 *			the nearest neighbor field to vote with
	 * \param params
	 *			the voting parameters
	 * \return the voted picture
	 */
	template <int channels, typename Patch, typename Scalar, int K, int RowMajor = true>
	Image disparity_vote(const PixelContainer<channels, Patch, Scalar, K> &data, const Filter &filter,
        const Field2D<RowMajor> &field, float noisePercentile = 0.975f) {
        
        typedef typename Patch::point PixelLoc;
        // typedef typename PixelLoc::vec Disparity;
        typedef Vec2f Disparity;
        
        // create workspace
        const Frame2D<Point2i, true> &frame = data.frame();
		Image vote = Image::zeros(frame.size.rows, frame.size.cols, IM_32FC(2)); // u+v
        
        // compute CDF of absolute disparity
        std::vector<float> disp;
        for(const Point2i &i : field){
            for(int k = 0; k < K; ++k){
                const Patch &p = data.patch(i, k);
                Disparity d = voting::disparity(i, p);
                disp.push_back(std::sqrt(d.dot(d)));
            }
        }
        voting::CDF cdf(disp, 200);
        float maxDisparity = cdf.percentile(noisePercentile);
        std::cout << "maxDisparity = " << maxDisparity << "\n";
        std::cout << "percentiles:";
        for(int i = 0; i < 100; i += 5){
            std::cout << i << "%=" << cdf.percentile(i / 100.0f) << ", ";
        }
        std::cout << "\n";
        
        // TODO use distance for the weight

		// for each pixel of the workspace
        for(const Point2i &i : frame){
            Disparity &disparity = vote.at<Disparity>(i);
            Scalar weight = 0;
            // use weighted average of overlapping patch pixels (kN^2 of them)
            for(const Point2i &p : data.overlap(i)){
                const Patch &patch = data.patch(p);
                const Point2i b = i - p;
                Disparity patchDisp = voting::disparity(p, patch);
                float d = std::sqrt(patchDisp.dot(patchDisp));
                if(d > maxDisparity){
                    patchDisp[0] = std::max(-maxDisparity, std::min(maxDisparity, patchDisp[0]));
                    patchDisp[1] = std::max(-maxDisparity, std::min(maxDisparity, patchDisp[1]));
                }
                disparity += patchDisp * filter[b.y][b.x];
                weight += filter[b.y][b.x];
            }
            if(weight > 1e-8){
                disparity *= 1.0 / weight;
            }
            // clamp it if it is above what we tolerate
            float d = std::sqrt(disparity.dot(disparity));
            if(d > maxDisparity){
                disparity[0] = std::max(-maxDisparity, std::min(maxDisparity, disparity[0]));
                disparity[1] = std::max(-maxDisparity, std::min(maxDisparity, disparity[1]));
            }
        }
		return vote;
	}
    
}

#endif	/* DISPARITY_VOTE_H */

