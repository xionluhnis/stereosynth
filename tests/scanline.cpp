#include "scanline.h"
#include "algebra.h"

using namespace pm;

template < typename T >
struct Increment {
	Grid2D<T> *grid;
	bool operator ()(const Point2i &i, bool) {
		grid->at(i.y, i.x) += 1;
        return true;
	}
	
	Increment(Grid2D<T> *g) : grid(g){}
};

template < typename T >
struct IncrementID {
	Grid2D<T> *grid;
	int id;
	bool operator ()(const Point2i &i, bool rev) {
		if(!rev){
			grid->at(i.y, i.x) += id++;
		} else {
			grid->at(i.y, i.x) -= --id;
		}
        return true;
	}
	
	IncrementID(Grid2D<T> *g) : grid(g), id(0){}
};

/**
 * Test to check that the scanline interface works as expected
 */
int main(){
	
	Grid2D<int> g(100, 100, true); // set everything to zero by default (true)
	
	// 0: the grid should be null
	for(int y = 0; y < 100; ++y) {
		for(int x = 0; x < 100; ++x) {
			assert(g.at(y, x) == 0 && "Invalid initial grid. Not zero!");
		}
	}
	
	// 1: go over all pixels, they should be zero
	scanline(g, 7, Increment<int>(&g));
	for(int y = 0; y < 100; ++y) {
		for(int x = 0; x < 100; ++x) {
			assert(g.at(y, x) == 7 && "Increment did not work!");
		}
	}
	g.clear();
	// 1b: Grid2D.clear() should reset everything to 0
	for(int y = 0; y < 100; ++y) {
		for(int x = 0; x < 100; ++x) {
			assert(g.at(y, x) == 0 && "Clear did not work. Not zero!");
		}
	}
	
	// 2: go over all pixels, assign up / down id (even iterations)
	scanline(g, 8, IncrementID<int>(&g));
	for(int y = 0; y < 100; ++y) {
		for(int x = 0; x < 100; ++x) {
			assert(g.at(y, x) == 0 && "IncrementID failed!");
		}
	}
	g.clear();
	
	// 3: simple linear id
	scanline(g, 1, IncrementID<int>(&g));
	for(int y = 0, id = 0; y < 100; ++y) {
		for(int x = 0; x < 100; ++x, ++id) {
			assert(g.at(y, x) == id && "IncrementID failed!");
		}
	}
	g.clear();
	
	return 0;
}