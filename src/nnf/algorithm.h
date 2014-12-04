/* 
 * File:   algorithm.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 21, 2014, 11:18 AM
 */

#ifndef ALGORITHM_H
#define	ALGORITHM_H

#include <functional>

namespace pm {
    
    /**
     * Algorithm sequence wrapper
     */
    struct Algorithm {
        typedef std::function<uint(const Point2i &, bool)> AlgorithmPart;
        uint operator()(const Point2i &i, bool rev) {
            uint res = 0;
            for(AlgorithmPart &p : seq){
                res += p(i, rev);
            }
            return res;
        }

        Algorithm(){}

        Algorithm &operator <<(AlgorithmPart p){
            seq.push_back(p);
            return *this;
        }
    private:
        std::vector<AlgorithmPart> seq;
        friend class VerboseAlgorithm;
    };
    
    /**
     * Algorithm sequence wrapper with result verbosity
     */
    struct VerboseAlgorithm {
        typedef std::function<uint(const Point2i &, bool)> AlgorithmPart;
        uint operator()(const Point2i &i, bool rev) {
            uint res = 0;
            for(uint j = 0, n = seq.size(); j < n; ++j){
                AlgorithmPart &p = seq[j];
                uint c = p(i, rev);
                res += c; // total count
                results[j] += c; // per-algorithm count
            }
            return res;
        }
        VerboseAlgorithm(const Algorithm &algo){
            seq.resize(algo.seq.size());
            std::copy(algo.seq.begin(), algo.seq.end(), seq.begin());
            std::fill(results.begin(), results.end(), 0);
        }
        VerboseAlgorithm(){}

        VerboseAlgorithm &operator <<(AlgorithmPart p){
            seq.push_back(p);
            results.push_back(0);
            return *this;
        }
        
        const std::vector<size_t> &counts() const {
            return results;
        }
        
    private:
        std::vector<AlgorithmPart> seq;
        std::vector<size_t> results;
    };
    
    /**
     * Diary recording convergence over iterations
     */
    struct ConvergenceDiary {
        typedef std::vector<size_t> sequence;
        
        uint operator()(int iter, bool rev) {
            const std::vector<size_t> &sums = algorithm->counts();
            uint n = sums.size();
            if(convergence.empty()){
                convergence.resize(n);
            }
            for(uint j = 0; j < n; ++j){
                size_t curr = sums[j];
                size_t prev = convergence[j].empty() ? 0 : convergence[j].back();
                assert(prev <= curr && "Convergence decrease?");
                convergence[j].push_back(curr - prev);
            }
            return 0;
        }
        
        const std::vector<sequence> &diary() const {
            return convergence;
        }
        
        ConvergenceDiary(const VerboseAlgorithm *algo) : algorithm(algo) {
        }
        
    private:
        const VerboseAlgorithm *algorithm;
        std::vector< sequence > convergence;
    };
    
}

#endif	/* ALGORITHM_H */

