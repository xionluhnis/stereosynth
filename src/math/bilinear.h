/* 
 * File:   BilinearImage.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 3, 2014, 3:56 PM
 */

#ifndef BILINEARIMAGE_H
#define	BILINEARIMAGE_H

#include "../algebra.h"

namespace pm {
    
    template <typename T, typename S>
    inline T bilinearLookup(const Mat &img, const Point<S> &p) {
        // simple bilinear interpolation
        // @see http://en.wikipedia.org/wiki/Bilinear_interpolation
        // the pixel on the left
        int x0 = std::floor(p.x); int x1 = std::ceil(p.x);
        int y0 = std::floor(p.y); int y1 = std::ceil(p.y);
        // the ratios for the subpixel location
        // /!\ x - floor(x) => ratio of ceiling sidehe
        S rightRatio	= p.x - x0;
        S leftRatio		= S(1.0) - rightRatio;
        S bottomRatio	= p.y - y0;
        S topRatio		= S(1.0) - bottomRatio;
        // neighboring pixels
        const T &topLeft		= img.at<T>(y0, x0);
        const T &bottomLeft		= y1 == y0 ? topLeft : img.at<T>(y1, x0);
        const T &topRight		= x1 == x0 ? topLeft : img.at<T>(y0, x1);
        const T &bottomRight	= img.at<T>(y1, x1);
        return		topLeft		* leftRatio		* topRatio
                +	bottomLeft	* leftRatio		* bottomRatio
                +	topRight	* rightRatio	* topRatio
                +	bottomRight * rightRatio	* bottomRatio;
    }
    
    
    template <typename S = float>
    struct BilinearMat : public Mat {
        
        typedef Point<S> point;
        typedef typename point::vec vec;
        
        BilinearMat() : Mat(){
		}
		
		BilinearMat(const Mat &img) : Mat(img){
		}
        
        template <typename T>
        inline T at(const point &p) const {
            Point2i i(p);
            if(p == point(i)){
                return Mat::at<T>(p);
            }
            return bilinearLookup<T, S>(*this, p);
        }
        template <typename T>
        inline T at(const point &p) {
            Point2i i(p);
            if(p == point(i)){
                return Mat::at<T>(p);
            }
            return bilinearLookup<T, S>(*this, p);
        }
        
        template <typename T>
        inline const T &at(const Point2i &p) const {
            return Mat::at<T>(p);
        }
        template <typename T>
        inline T &at(const point &p) {
            T &v = Mat::at<T>(p);
            return v;
        }
        
		inline bool contains(const point &p) const {
			return p.x >= 0 && p.y >= 0 && p.x <= width - 1 && p.y <= height - 1;
		}
    };
    
    typedef BilinearMat<float> BilinearMatF;
    typedef BilinearMat<double> BilinearMatD;
    
}

#endif	/* BILINEARIMAGE_H */

