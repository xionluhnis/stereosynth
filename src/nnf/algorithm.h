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
    template < typename Index = Point2i >
    struct AlgorithmSequence {
        typedef std::function<uint(const Index &, bool)> AlgorithmPart;
        uint operator()(const Index &i, bool rev) {
            uint res = 0;
            for(AlgorithmPart &p : seq){
                res += p(i, rev);
            }
            return res;
        }

        AlgorithmSequence(){}

        AlgorithmSequence &operator <<(AlgorithmPart p){
            seq.push_back(p);
            return *this;
        }
    private:
        std::vector<AlgorithmPart> seq;
        friend class VerboseAlgorithm;
    };
    // the main algorithm type
    typedef AlgorithmSequence<Point2i> Algorithm;
    typedef AlgorithmSequence<int> PostSequence;
    
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
            // function sequence
            seq.resize(algo.seq.size());
            std::copy(algo.seq.begin(), algo.seq.end(), seq.begin());
            // result sequence
            results.resize(algo.seq.size());
            std::fill(results.begin(), results.end(), 0); // set to zero
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
        typedef std::vector<size_t> Sequence;
        typedef std::vector<Sequence> Data;
        
        uint operator()(int iter, bool rev) {
            const std::vector<size_t> &sums = algorithm->counts();
            uint n = sums.size();
            if(data->empty()){
                data->resize(n);
            }
            for(uint j = 0; j < n; ++j){
                Sequence &seq = data->at(j);
                size_t curr = sums[j];
                size_t last = seq.empty() ? 0 : seq.back();
                assert(last <= curr && "Convergence decrease?");
                data->at(j).push_back(curr - last);
            }
            return 0;
        }
        
        ConvergenceDiary(const VerboseAlgorithm *algo, Data *d) : algorithm(algo), data(d) {
            assert(data && "Null diary!");
        }
        
    private:
        const VerboseAlgorithm *algorithm;
        Data *data;
    };
    
}

#endif	/* ALGORITHM_H */

