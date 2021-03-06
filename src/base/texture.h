/* 
 * File:   texture.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 21, 2014, 4:20 PM
 */

#ifndef TEXTURE_H
#define	TEXTURE_H

#include "../math/mat.h"

#include <boost/shared_array.hpp>
#include <iostream>
#include <new>

namespace pm {

	/**
	 * Type of texture boundary model
	 */
	enum BoundaryModel {
		/// No out-of-bound pixel
		Unchecked = 0,
		/// Safe checking (triggers an exception if out of bound)
		Checked = 1,
		/// The boundary pixels are repeated out of the boundaries
		Same = 2,
		/// The texture wraps around a torus
		Toroidal = 3
	};

	/**
	 * The type of pixel interpolation
	 */
	enum PixelInterpolation {
		NearestNeighbor = 0,
		Bilinear = 1
	};

	struct Texture : public Mat {

		Texture() : Mat(), model(Same), interp(Bilinear) {}
		explicit Texture(const Mat &m, int depth = 0, 
			BoundaryModel bm = Unchecked, 
			PixelInterpolation pi = Bilinear) : Mat(m), model(bm), interp(pi) {
			if(depth > 0) {
				stack.reset(new Image[depth]); // FIXME should we call the constructor? where do we initialize them?
			}
		}

		/// project a pixel coordinate according to the boundary model
		inline void project(int &y, int &x) const {
			switch (model) {
				case Checked:
                    assert(x >= 0 && x < width && y >= 0 && y < height && "Texture out of bound access");
					break;
				case Same:
					x = std::max(0, std::min(width - 1, x));
					y = std::max(0, std::min(height - 1, y));
					break;
				case Toroidal:
					if (x < 0) x += width;
					else if (x >= width) x = x % width;
					if (y < 0) y += height;
					else if (y >= height) y = y % height;
					break;
				// case Unchecked:
				default:
					break;
			}
		}

		//! Element access
		template <typename T>
        inline const T & at(int y, int x, int depth = 0) const {
			project(y, x);
			if(depth == 0)
                return Mat::at<T>(y, x);
            else
                return stack[depth - 1].at<T>(y, x);
		}
		template <typename T>
        inline T & at(int y, int x, int depth = 0) {
			project(y, x);
			if(depth == 0)
                return Mat::at<T>(y, x);
            else
                return stack[depth - 1].at<T>(y, x);
		}

		//! Element access by point location
		template <typename T> inline const T & at(const Point<int> &p) const {
			return at<T>(p.y, p.x);
		}
		template <typename T> inline T & at(const Point<int> &p) {
			return at<T>(p.y, p.x);
		}

		//! Element access by continuous point location
		template <typename T> inline T at(const Point<float> &p, int depth = 0) const {
			if (interp == NearestNeighbor) {
				return at<T>(round(p.y), round(p.x), depth);
			}
						
			// simple bilinear interpolation
			// @see http://en.wikipedia.org/wiki/Bilinear_interpolation
			// the pixel on the left
			int x0 = std::floor(p.x);
			int y0 = std::floor(p.y);
			// the ratios for the subpixel location
			// /!\ x - floor(x) => ratio of ceiling sidehe
			float rightRatio	= p.x - x0;
			float leftRatio		= 1.0f - rightRatio;
			float bottomRatio	= p.y - y0;
			float topRatio		= 1.0f - bottomRatio;
			// neighboring pixels
			const T &topLeft		= at<T>(y0,		x0,		depth);
			const T &bottomLeft		= at<T>(y0 + 1,	x0,		depth);
			const T &topRight		= at<T>(y0,		x0 + 1, depth);
			const T &bottomRight	= at<T>(y0 + 1, x0 + 1, depth);
			return		topLeft		* leftRatio		* topRatio
					+	bottomLeft	* leftRatio		* bottomRatio
					+	topRight	* rightRatio	* topRatio
					+	bottomRight * rightRatio	* bottomRatio;
		}

		//! Multiple depth access
		template <typename T, typename S> inline const T & at(const MultiDepthPoint<S> &p) const {
			return at<T>(p.y, p.x, p.depth);
		}
		template <typename T> inline T & at(const MultiDepthPoint<int> &p) {
			return at<T>(p.y, p.x, p.depth);
		}
		template <typename T> inline T at(const MultiDepthPoint<float> &p) const {
			return at<T>(p, p.depth);
		}

		/// The boundary model
		BoundaryModel model;
		PixelInterpolation interp;
		/// The multi-resolution texture stack
		boost::shared_array<Image> stack;
	};
}

#endif	/* TEXTURE_H */
