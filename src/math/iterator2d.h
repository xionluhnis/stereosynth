/* 
 * File:   iterator2d.h
 * Author: xion
 *
 * Created on November 15, 2014, 9:31 PM
 */

#ifndef ITERATOR2D_H
#define	ITERATOR2D_H

namespace pm {

	/**
	 * Iterator over a 2D grid
	 */
	template < typename Index, bool RowMajor = true >
	struct Iterator2D {
		typedef Iterator2D<Index, RowMajor> this_type;
		typedef Index this_value;
        
        static const bool ROW_MAJOR = true;
        static const bool COL_MAJOR = false;
        
        Iterator2D() : i0(0), i1(0), start(0), end(0), dir0(0), dir1(0) {}

		explicit Iterator2D(int x0, int x1, int s0, int e0, int d0, int d1)
		: i0(x0), i1(x1), start(s0), end(e0), dir0(d0), dir1(d1) {
		}

		inline int x0() const {
			return i0;
		}

		inline int x1() const {
			return i1;
		}

		inline this_type & operator ++() {
			i0 += dir0;
			if (i0 == end) {
				i0 = start;
				i1 += dir1;
			}
			return *this;
		}

		inline bool operator==(const this_type &it) const {
			return it.i0 == i0 && it.i1 == i1;
		}
        inline bool operator !=(const this_type &it) const {
            return it.i0 != i0 || it.i1 != i1;
        }

		operator this_value() const {
			return RowMajor ? this_value(i0, i1) : this_value(i1, i0);
		}

		inline this_value operator*() const {
			return this_value(*this);
		}

	private:
		int i0, i1;
		int start, end;
		int dir0, dir1;
	};

	/**
	 * Adaptor that creates an iterator over a 2D grid
	 */
	template < typename Index, bool RowMajor = true >
	struct Iterable2D {
		typedef Index index;
		typedef Iterator2D<Index, RowMajor> iterator;

		iterator begin() const {
			return iterator(
				0, 0,
				0, size0(),
				1, 1
			);
		}

		iterator end() const {
			return iterator(
				0, size1(),
				0, size0(),
				1, 1
			);
		}

		iterator rbegin() const {
			return iterator(
				size0() - 1, size1() - 1,
				size0() - 1, -1,
				-1, -1
			);
		}

		iterator rend() const {
			return iterator(
				size0() - 1, -1,
				size0() - 1, -1,
				-1, -1
			);
		}
        
        virtual int size0() const = 0;
        virtual int size1() const = 0;
	};
    
    // -------------------------------------------------------------------------
    // Iteration frame size
    //
    struct FrameSize {
        union {
            int width;
            int cols;
        };
        union {
            int height;
            int rows;
        };
        explicit FrameSize(int w = 0, int h = 0) : width(w), height(h){}
        
        inline FrameSize shrink(int margin) const {
            return FrameSize(width - margin, height - margin);
        }
        inline FrameSize transpose() const {
            return FrameSize(height, width);
        }
    };
    
    // -------------------------------------------------------------------------
    // Iterable2D that can be instantiated without any virtual function need
    //
    template < typename Index, bool RowMajor = true >
    struct Frame2D {
        typedef Index index;
		typedef Iterator2D<Index, RowMajor> iterator;
        
        const FrameSize size;

		iterator begin() const {
			return iterator(
				0, 0,
				0, size.width,
				1, 1
			);
		}

		iterator end() const {
			return iterator(
				0, size.height,
				0, size.width,
				1, 1
			);
		}

		iterator rbegin() const {
			return iterator(
				size.width - 1, size.height - 1,
				size.width - 1, -1,
				-1, -1
			);
		}

		iterator rend() const {
			return iterator(
				size.width - 1, -1,
				size.width - 1, -1,
				-1, -1
			);
		}
        
        Frame2D(int s0, int s1) : size(RowMajor ? s0 : s1, RowMajor ? s1 : s0){}
        Frame2D(const FrameSize &s) : size(RowMajor ? s : s.transpose()){}
    };

}

#endif	/* ITERATOR2D_H */

