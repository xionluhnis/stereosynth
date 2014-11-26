#include "algebra.h"

using namespace pm;

/**
 * Test to check that the scanline interface works as expected
 */
int main(){
	
	// default constructor
	Bounds2i b(0);
	assert(b.start == b.end && b.start == Vec2i(0, 0) && "Invalid default bounds.");
	
	// vector constructor
	b = Bounds2i(Vec2i(0, 0), Vec2i(1, 1));
	assert(b.start == Vec2i(0, 0) && b.end == Vec2i(1, 1) && "Invalid vector bounds.");
	
	// intersection with larger bounds
	Bounds2i c = b & Bounds2i(Vec2i(0, 0), Vec2i(2, 2));
	assert(b == c && "Intersection with container is wrong.");
	
	// intersection with smaller bounds
	Bounds2i d(Vec2i(1, 1), Vec2i(1, 1));
	Bounds2i e = b & d;
	assert(e.range() == Vec2i(0, 0) && "Singleton range is not null.");
	assert(e.start == e.end && e.start == Vec2i(1, 1) && "Intersection is wrong.");
	
	// window constructor
	Bounds2i f(Vec2i(5, 5), 5);
	assert(f.start == Vec2i(0, 0) && f.end == Vec2i(10, 10) && "Invalid window constructor.");
	
	return 0;
}