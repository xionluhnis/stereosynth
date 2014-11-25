/* 
 * File:   point.h
 * Author: xion
 *
 * Created on November 15, 2014, 9:22 PM
 */

#ifndef POINT_H
#define	POINT_H

#include "defs.h"
#include "vec.h"

namespace pm {
	
	/**
	 * Simple 2d point
	 */
	template <typename T>
	struct Point {
		typedef T scalar;
		typedef Vec<T, 2> vec;
		typedef Point<T> point;
		enum {
			typeDepth = DataDepth<scalar>::value,
			channels = 2,
			type = IM_MAKETYPE(typeDepth, channels),
			exact = DataDepth<scalar>::exact
		};
		Point() : x(0), y(0) {}
		Point(T a, T b) : x(a), y(b) {}
		template <typename T2>
		explicit Point(const Point<T2> &p) : x(p.x), y(p.y) {}
        Point(const vec &v) : x(v[0]), y(v[1]){}
        
        inline point transpose() const {
            return point(y, x);
        }
		
		inline T dot(const point &p) const {
			return p.x * x + p.y * y;
		}
		inline T sqNorm() const {
			return dot(*this);
		}
		inline point operator +(const point &p) const {
			return point(x + p.x, y + p.y);
		}
		inline point operator -(const point &p) const {
			return point(x - p.x, y - p.y);
		}
		inline point operator *(T f) const {
			return point(x * f, y * f);
		}
		inline point operator -() const {
			return point(-x, -y);
		}
		inline point mult(const point &p) const {
			return point(x * p.x, y * p.y);
		}
		template <typename I>
		inline point within(const I *image) const {
			return point(
					std::max(T(0), std::min(T(image->width - 1), x)),
					std::max(T(0), std::min(T(image->height - 1), y))
					);
		}
		inline operator Point<int>() const {
			return Point<int>(round(x), round(y));
		}
		inline Point<int> floor() const {
			return Point<int>(std::floor(x), std::floor(y));
		}
		inline Point<int> ceil() const {
			return Point<int>(std::ceil(x), std::ceil(y));
		}
		inline point abs() const {
			return point(std::abs(x), std::abs(y));
		}
		inline bool isOrigin() const {
			return x == 0 && y == 0;
		}
		inline static Point<T> min(const Point<T> &p1, const Point<T> &p2) {
			return Point<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
		}
		inline static Point<T> max(const Point<T> &p1, const Point<T> &p2) {
			return Point<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
		}
		inline T area() const {
			return x * y;
		}
		
		operator vec() const {
			return vec(x, y);
		}
		
		T x;
		T y;
	};
	
	typedef Point<int> Point2i;
	typedef Point<float> Point2f;
	typedef Point<double> Point2d;
	
}

#endif	/* POINT_H */

