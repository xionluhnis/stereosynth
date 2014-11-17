/* 
 * File:   abstractgrid.h
 * Author: xion
 *
 * Created on November 15, 2014, 9:25 PM
 */

#ifndef GRID2D_H
#define	GRID2D_H

#include "iterator2d.h"
#include "point.h"

#include <boost/shared_ptr.hpp>

namespace pm {
	
	/**
	 * Simple 2D grid for a given type of data
	 * 
	 * @see Mat for dynamic types (specified only by bit size)
	 */
	template <typename T, typename Index = Point2i, bool RowMajor = true>
    class Grid2D : public Iterable2D<Index, RowMajor>{
    public:
        union {
            int rows;
            int height;
        };
        union {
            int cols;
            int width;
        };
        
        static const bool ROW_MAJOR = true;
        static const bool COL_MAJOR = false;
        
        Grid2D() : height(0), width(0), data(){}
        Grid2D(int h, int w, bool init = false) : height(h), width(w), data() {
            T *ptr = NULL;
            if(init){
                ptr = new T[h * w]();
            } else {
                ptr = new T[h * w];
            }
            if(ptr != NULL) data.reset(ptr);
        }
        
        inline bool empty() const {
            return data;
        }
		
		inline void clear() {
			std::fill(ptr(), ptr() + rows * height, T());
		}
        
        inline T *ptr() {
            return data.get();
        }
        inline const T *ptr() const {
            return data.get();
        }
        
        inline int idx(int y, int x) const {
            if(RowMajor)
                return y * width + x;
            else
                return x * height + y;
        }
        
        inline T &at(int y, int x) {
            return ptr()[idx(y, x)];
        }
        inline const T &at(int y, int x) const {
            return ptr()[idx(y, x)];
        }
		
        // iterable implementation
		virtual int size0() const {
            return width;
        }
        virtual int size1() const {
            return height;
        }
        
    private:
        boost::shared_ptr<T> data;
    };
	
}

#endif	/* GRID2D_H */

