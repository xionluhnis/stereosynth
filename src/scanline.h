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
        for(auto i : grid.reversed(rev)){
            // filter index
            if(IterationFilter(i)) continue;
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

struct Iterator2D {
	
	explicit Iterator2D(int s, int d0, int d1, int x0, int x1)
	: i0(x0), i1(x1), step(s), dir0(d0), dir1(d1) {
	}
	
	inline int x0() const {
		return i0;
	}
	inline int x1() const {
		return i1;
	}
	
	inline Iterator2D & operator ++() {
		i0 += dir0;
		if(i0 == step){
			i0 = start;
			i1 += dir1;
		}
		return *this;
	}
	
	inline bool operator==(const Iterator2D &it) const {
		return it.i0 == i0 && it.i1 == i1;
	}
	
	template <typename T>
	T as() const {
		return T(i0, i1);
	}
private:
	int i0, i1;
	int start;
	int step;
	int dir0, dir1;
};

template < bool RowMajor >
struct Grid2D {
	typedef Iterator2D index;
	typedef Iterator2D iterator;
	typedef Iterator2D const_iterator;
	typedef Iterator2D reverse_iterator;
	
	Grid2D(int s0, int s1) : size0(s0), size1(s1){}
	
	const_iterator begin() const;
	const_iterator end() const;
	iterator rbegin() const;
	iterator rend() const;
	
private:
	int size0, size1;
};


#endif	/* SCANLINE_H */

