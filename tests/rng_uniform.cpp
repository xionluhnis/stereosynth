#include "sampling/uniform.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace pm;

double sum(const int *arr, int count) {
    double sum = 0.0;
    for(int i = 0; i < count; ++i){
        sum += i * arr[i];
    }
    return sum;
}

/**
 * Test to check that the scanline interface works as expected
 */
int main(){
	
    RNG rand = unif01;
    
    // bins
    int disc_bins[10] = { 0 };
    int cont_bins[10] = { 0 };
    
    // we do it many times to check
    // ... this is not perfect at all!
    int N = 100000;
	for(int it = 0; it < N; ++it){
        
        // check discrete version
        int n = uniform(rand, 100, 110);
        assert(n >= 100 && n <= 110 && "Discrete uniform is out of bounds!");
        
        // check continuous version
        float x = uniform(rand, 100.0f, 110.0f);
        assert(x >= 100.0f && x <= 110.0f && "Continuous uniform is out of bounds!");
        
        // check vector versions
        Vec2i v = uniform(rand, Vec2i(50, 100), Vec2i(60, 110));
        assert(v[0] >= 50 && v[0] <= 60 && v[1] >= 100 && v[1] <= 110 && "Discrete vector uniform is out of bounds!");
        
        Vec2f w = uniform(rand, Vec2f(50, 100), Vec2f(60, 110));
        assert(w[0] >= 50 && w[0] <= 60 && w[1] >= 100 && w[1] <= 110 && "Continuous vector uniform is out of bounds!");
        
        // building distribution
        disc_bins[uniform<int>(rand, 0, 9)] += 1;
        cont_bins[std::min(9, int(uniform<float>(rand, 0.0f, 10.0f)))] += 1;
    }
    // mean = (a+b)/2
    double disc_m = sum(disc_bins, 10) / N;
    assert(std::abs(disc_m - 4.5) < 1 && "Discrete mean far from expected!");

    double cont_m = sum(cont_bins, 10) / N;
    assert(std::abs(cont_m - 5.0) < 1 && "Continuous mean far from expected!");
	
	return 0;
}