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
    typename Algorithm = NoOp<typename Grid::Index>,
    typename IterationFilter = NoOp<typename Grid::Index>,
    typename IterationEnd = NoOp<typename Grid::Index>
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
        if(done){
            break;
        }
    }
}


#endif	/* SCANLINE_H */

