/* 
 * File:   mat.h
 * Author: xion
 *
 * Created on November 15, 2014, 9:18 PM
 */

#ifndef MATH_MAT_H
#define	MATH_MAT_H

#include "defs.h"
#include "iterator2d.h"
#include "point.h"

#include <boost/shared_ptr.hpp>

namespace pm {
	
	/**
	 * Matrix data pointer
	 */
	typedef boost::shared_ptr<byte> DataPtr;
	
#ifndef SAFE_MAT
#define SAFE_MAT 0
#endif

	/**
	 * Image matrix representation
	 */
	struct Mat : public Iterable2D<Point2i, true> {
		union {
			int rows;
			int height;
		};
		union {
			int cols;
			int width;
		};
        
		//! returns the type of the matrix elements
		inline int type() const {
			return flags;
		}
		//! returns the depth of the matrix elements
		inline int depth() const {
			return IM_MAT_DEPTH(flags);
		}
		//! returns the number of channels in each matrix element
		inline int channels() const {
			return IM_MAT_CN(flags);
		}
		
		Mat() : flags(IM_UNKNOWN){
		}
		
		Mat(int h, int w, int dataType) : height(h), width(w), flags(dataType){
			create(IM_SIZEOF(dataType));
		}
        
        Mat(int h, int w, size_t elemSize, int channels) : height(h), width(w), flags(IM_MAKETYPE(IM_USRTYPE, channels)){
			create(elemSize);
		}
        
    protected:
        
        void create(size_t elemSize){
            size_t byteCount = elemSize * height * width;
			if(byteCount > 0){
				byte *content = new byte[byteCount];
				data.reset(content);
				step[0] = elemSize;
				step[1] = width * elemSize;
			} else {
				std::cerr << "Matrix with datasize=" << elemSize << ", datatype=" << flags << "\n";
				step[0] = step[1] = 0;
			}
        }
        
    public:
        
        inline int elemSize() const {
            return step[0];
        } 
		
		inline bool empty() const {
			return !data;
		}
        
        virtual int size0() const {
            return width;
        }
        virtual int size1() const {
            return height;
        }
		
		inline static Mat zeros(int rows, int cols, int type) {
			Mat m(rows, cols, type);
			int elemSize = IM_SIZEOF(type);
			int byteCount = elemSize * rows * cols;
			byte *ptr = m.ptr();
			std::fill(ptr, ptr + byteCount, 0);
			return m;
		}
		
		//! Direct pointer access
		inline byte *ptr() {
			return data.get();
		}
		
		//! Pointer access
		template <typename T>
		inline const T *ptr(int y, int x) const {
#if SAFE_MAT
			if(x < 0 || x >= width || y < 0 || y >= height) {
				std::cout << y << "/" << x << "\n";
				mexErrMsgIdAndTxt("MATLAB:img:ptr", "Out of image bounds!");
			}
#endif
			const byte *ref = data.get();
			return reinterpret_cast<const T*>(ref + y * step[1] + x * step[0]);
		}
		template <typename T>
		inline T *ptr(int y, int x) {
#if SAFE_MAT
			if(x < 0 || x >= width || y < 0 || y >= height) {
				std::cout << y << "/" << x << "\n";
				mexErrMsgIdAndTxt("MATLAB:img:ptr", "Ref out of image bounds!");
			}
#endif
			byte *ref = data.get();
			return reinterpret_cast<T*>(ref + y * step[1] + x * step[0]);
		}
		
		//! Element access
		template <typename T>
		inline const T &at(int y, int x) const {
			return *ptr<T>(y, x);
		}
		template <typename T>
		inline T &at(int y, int x) {
			return *ptr<T>(y, x);
		}
        
        template <typename T>
        inline const T &at(const Point2i &p) const {
            return at<T>(p.y, p.x);
        }
        template <typename T>
        inline T &at(const Point2i &p) {
            return at<T>(p.y, p.x);
        }
		
	private:
		int flags;
		DataPtr data;
		int step[2];
	};
	
	/**
	 * Image type
	 */
	typedef Mat Image;

}

#endif	/* MATH_MAT_H */

