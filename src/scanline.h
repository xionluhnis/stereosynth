/*******************************************************************************
 * scanline.h - scanline traversal algorithm
 *******************************************************************************
 * Add license here...
 *******************************/

#ifndef SCANLINE_H
#define	SCANLINE_H

struct NoOp {
    /**
     * Iteration per index
     */
	template <typename T>
    bool operator()(const T &, bool){
        // do ... nothing
        return false; // we can shortcircuit directly
    }
};

template <
	typename Grid,
    typename Algorithm = NoOp,
    typename IterationFilter = NoOp,
    typename IterationEnd = NoOp
>
void scanline(
	Grid &grid, unsigned int numIters,
	Algorithm &algo = NoOp(),
	IterationFilter &filter = NoOp(),
	IterationEnd &iterEnd = NoOp()
){
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
		for(; it != end; ++it){
			const typename Grid::index i = *it; // we don't want it to change in between!
			// filter index
			if(filter(i)) continue;
			// execute improvement
			if(algo(i, rev)){
			   done = false; // the update was successful => more to do
			}
		}
		rev = !rev; // reverse scanline order
		iterEnd(iter);
		// potential shortcut
		if(done){
			break;
		}
	}
}

#endif	/* SCANLINE_H */

