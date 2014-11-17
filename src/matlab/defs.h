/* 
 * File:   defs.h
 * Author: akaspar
 *
 * Created on November 17, 2014, 5:06 PM
 */

#ifndef MATLAB_DEFS_H
#define	MATLAB_DEFS_H

#include "../math/defs.h"

#include <mex.h>

namespace pm {
    
    ////////////////////////////////////////////////////////////////////////////
    ///// Image Type to mxClassID //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    inline mxClassID classOf(int dataType) {
        int depth = IM_MAT_DEPTH(dataType);
        switch (depth) {
            case IM_8U: return mxUINT8_CLASS;
            case IM_8S: return mxINT8_CLASS;
            case IM_32S: return mxINT32_CLASS;
            case IM_32F: return mxSINGLE_CLASS;
            case IM_64F: return mxDOUBLE_CLASS;
            default:
                mexErrMsgIdAndTxt("MATLAB:mex:classOf", "Unsupported class!");
                return mxUNKNOWN_CLASS;
        }
    }
    inline mxClassID classOf(const Image &img) {
        return classOf(img.depth());
    }

    inline int depthOf(mxClassID c) {
        switch(c) {
            case mxUINT8_CLASS: return IM_8U;
            case mxINT8_CLASS: return IM_8S;
            case mxINT32_CLASS: return IM_32S;
            case mxSINGLE_CLASS: return IM_32F;
            case mxDOUBLE_CLASS: return IM_64F;
            case mxLOGICAL_CLASS: return IM_8U;
            default:
                mexErrMsgIdAndTxt("MATLAB:mex:depthOf", "Unsupported depth!");
                return -1;
        }
    }
    inline int depthOf(const mxArray *arr) {
        return depthOf(mxGetClassID(arr));
    }
    
    template <typename Scalar>
    mxClassID classID();
    
    template <>
    mxClassID classID<float>() {
        return mxSINGLE_CLASS;
    }

    template <>
    mxClassID classID<double>() {
        return mxDOUBLE_CLASS;
    }

    template <>
    mxClassID classID<unsigned char>() {
        return mxUINT8_CLASS;
    }

    template <>
    mxClassID classID<char>() {
        return mxINT8_CLASS;
    }
    
}

#endif	/* MATLAB_DEFS_H */

