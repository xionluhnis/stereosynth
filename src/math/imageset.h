/* 
 * File:   imageset.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on December 5, 2014, 2:40 PM
 */

#ifndef IMAGESET_H
#define	IMAGESET_H

#include "bilinear.h"
#include "mat.h"
#include "pointx.h"

#include <type_traits>
#include <boost/shared_array.hpp>

namespace pm {
    
    struct ImageSet {

		ImageSet() {}
		explicit ImageSet(size_t n) : N(n) {
			if(N > 0) {
				stack.reset(new Mat[N]());
			}
		}

		//! Element access
		template <typename T>
        inline const T & at(int y, int x, int index) const {
            return stack[index].at<T>(y, x);
		}
		template <typename T>
        inline T & at(int y, int x, int index) {
            return stack[index].at<T>(y, x);
		}
        
        //! discrete access
        template <typename T, typename S>
        inline typename std::enable_if<std::is_integral<S>::value, T>::type &at(const IndexedPoint<S> &i) {
            return at<T>(i.x, i.y, i.index);
        }
        template <typename T, typename S>
        inline const typename std::enable_if<std::is_integral<S>::value, T>::type &at(const IndexedPoint<S> &i) const {
            return at<T>(i.x, i.y, i.index);
        }
        
        //! continuous access
		template <typename T, typename S>
        inline typename std::enable_if<std::is_floating_point<S>::value, T>::type at(const IndexedPoint<S> &ip) const {
            typedef typename IndexedPoint<S>::base BasePoint;
            BasePoint p(ip); // without index
			Point2i i(p); // integer version
            if(p == BasePoint(i)){
                // our version is equivalent to a integer one, no need to interpolate
                return at<T>(i.x, i.y, ip.index);
            }
            // we need to interpolate
            return bilinearLookup<T, S>(stack[ip.index], p);
		}
        
        inline Mat &operator[](size_t i){
            return stack[i];
        }
        inline const Mat &operator[](size_t i) const {
            return stack[i];
        }
        inline size_t size() const {
            return N;
        }
        
    private:
		boost::shared_array<Mat> stack;
        size_t N;
	};
    
}

#endif	/* IMAGESET_H */

