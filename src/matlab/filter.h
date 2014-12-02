/* 
 * File:   filter.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 17, 2014, 5:29 PM
 */

#ifndef MATLAB_FILTER_H
#define	MATLAB_FILTER_H

#include "defs.h"
#include "../math/filter.h"

namespace pm {
    inline void mxLoadFilter(Filter &filter, const mxArray *arr, const char *s) {
        if (!mxIsNumeric(arr)) {
            mexErrMsgIdAndTxt("MATLAB:vote:wrongWeights",
                    "vote_weight should be an array of numbers!");
        }
        int m = mxGetM(arr);
        int n = mxGetN(arr);
        if (m == n && m == 1) {
            filter = Filter(filter.width, mxGetScalar(arr));
            return;
        }
        if (m != n || m != filter.width) {
            mexErrMsgIdAndTxt("MATLAB:filter:wrongSize", s);
        }
        switch (mxGetClassID(arr)) {
            case mxSINGLE_CLASS: {
                float *data = reinterpret_cast<float *> (mxGetData(arr));
                for (int y = 0; y < m; ++y) {
                    for (int x = 0; x < n; ++x) {
                        filter[y][x] = data[y + x * m];
                    }
                }
            }
                break;
            case mxDOUBLE_CLASS: {
                double *data = reinterpret_cast<double *> (mxGetData(arr));
                for (int y = 0; y < m; ++y) {
                    for (int x = 0; x < n; ++x) {
                        filter[y][x] = float(data[y + x * m]);
                    }
                }
            }
                break;
            default:
                mexErrMsgIdAndTxt("MATLAB:filter:type",
                        "Filter type must be single or double.");
                break;
        }
    }
}

#endif	/* MATLAB_FILTER_H */

