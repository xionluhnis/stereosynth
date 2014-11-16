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
	template <typename T>
    class Grid2D {
    public:
        union {
            int rows;
            int height;
        };
        union {
            int cols;
            int width;
        };
        
        Grid() : height(0), width(0), data(){}
        Grid(int h, int w, bool init = false) : height(h), width(w), data() {
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
        
        inline T &at(int y, int x) {
            return ptr()[y * width + x];
        }
        inline const T &at(int y, int x) const {
            return ptr()[y * width + x];
        }
		
		operator Iterable2D<Point2i>() const {
			return Iterable2D<Point2i>(width, height);
		}
        
    private:
        boost::shared_ptr<T> data;
    };
	
}

#endif	/* GRID2D_H */

