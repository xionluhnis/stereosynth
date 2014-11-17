/*******************************************************************************
 * scanline.h - scanline traversal algorithm
 *******************************************************************************
 * Add license here...
 *******************************/

#ifndef SCANLINE_H
#define	SCANLINE_H

template< typename T, bool Result = false >
struct NoOp {
    /**
     * Iteration per index
     */
    bool operator()(const T &, bool) const {
        // do ... nothing
        return Result;
    }
};

template <
	typename Grid,
    typename Algorithm = NoOp<typename Grid::index>,
    typename IterationFilter = NoOp<typename Grid::index>,
    typename IterationEnd = NoOp<unsigned int>
>
void scanline(Grid &grid, unsigned int numIters, Algorithm &algo, IterationFilter &filter, IterationEnd &iterEnd){
	bool rev = false;
	for(unsigned int iter = 0; iter < numIters; ++iter){
		bool done = true;
		// scanline traversal
		typename Grid::iterator it, end;
		if(!rev){
			it = grid.begin();
			end = grid.end();
		} else {
			it = grid.rbegin();
			end = grid.rend();
		}
		for(; !(it == end); ++it){
			const typename Grid::index i = *it; // we don't want it to change in between!
			// filter index
			if(filter(i, rev)) continue;
			// execute improvement
			if(algo(i, rev)){
			   done = false; // the update was successful => more to do
			}
		}
		iterEnd(iter, rev);
        rev = !rev; // reverse scanline order
		// potential shortcut
		if(done){
			break;
		}
	}
}

// one eluded argument
template <
	typename Grid,
    typename Algorithm = NoOp<typename Grid::index>,
    typename IterationFilter = NoOp<typename Grid::index>
>
void scanline(Grid &grid, unsigned int numIters, Algorithm &&algo, IterationFilter &&filter){
    NoOp<unsigned int> defaultIterEnd;
    scanline(grid, numIters, algo, filter, defaultIterEnd);
}

// two eluded arguments
template <
	typename Grid,
    typename Algorithm = NoOp<typename Grid::index>
>
void scanline(Grid &grid, unsigned int numIters, Algorithm &&algo){
    NoOp<typename Grid::index> noFilter;
    NoOp<unsigned int> defaultIterEnd;
    scanline(grid, numIters, algo, noFilter, defaultIterEnd);
}

#endif	/* SCANLINE_H */

