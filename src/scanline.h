/*******************************************************************************
 * scanline.h - scanline traversal algorithm
 *******************************************************************************
 * Add license here...
 *******************************/

#ifndef SCANLINE_H
#define	SCANLINE_H

template < typename Index >
struct NoOp {
    /**
     * Iteration per index
     */
    bool operator()(const Index &){
        // do ... nothing
        return false; // we can shortcircuit directly
    }
    /**
     * Iteration end
     */
    void operator()(unsigned int){
        // do ... nothing again
    }
};

template <
    typename Grid,
    typename Algorithm = NoOp<typename Grid::index>,
    typename IterationFilter = NoOp<typename Grid::index>,
    typename IterationEnd = NoOp<typename Grid::index>
>
void scanline(Grid &grid, unsigned int numIters){
    bool rev = false;
    for(unsigned int iter = 0; iter < numIters; ++iter){
        bool done = true;
        // scanline traversal
		iterator it, end;
		if(!rev){
			it = grid.begin();
			end = grid.end();
		} else {
			it = grid.rbegin();
			end = grid.rend();
		}
        for(; it != end; ++it){
            // filter index
            if(IterationFilter(it)) continue;
            // execute improvement
            if(Algorithm(i)){
               done = false; // the update was successful => more to do
            }
        }
        rev = !rev; // reverse scanline order
        IterationEnd(iter);
		// potential shortcut
        if(done){
            break;
        }
    }
}

template < typename Index, bool RowMajor = true >
struct Iterator2D {
	
	typedef Iterator<Index, RowMajor> this_type;
	typedef Index this_value;
	
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
		if(i0 == end){
			i0 = start;
			i1 += dir1;
		}
		return *this;
	}
	
	inline bool operator==(const this_type &it) const {
		return it.i0 == i0 && it.i1 == i1;
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

template < typename Index, bool RowMajor >
struct Grid2D {
	typedef Index index;
	typedef Iterator2D<Index, RowMajor> iterator;
	typedef iterator const_iterator;
	typedef iterator reverse_iterator;
	
	Grid2D(int s0, int s1) : size0(RowMajor ? s0 : s1), size1(RowMajor ? s1 : s0){}
	
	iterator begin() const {
		return iterator(
			0, 0,
			0, size0,
			1, 1
		);
	}
	iterator end() const {
		return iterator(
			0, size1, 
			0, size0,
			1, 1
		);
	}
	iterator rbegin() const {
		return iterator(
			size0 - 1, size1 - 1, 
			size0 - 1, -1, 
			-1, -1
		);
	}
	iterator rend() const {
		return iterator(
			size0 - 1, -1,
			size0 - 1, -1
			-1, -1
		);
	}
	
private:
	int size0, size1;
};


#endif	/* SCANLINE_H */

