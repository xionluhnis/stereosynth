/* 
 * File:   arguments.h
 * Author: akaspar
 *
 * Created on November 17, 2014, 5:14 PM
 */

#ifndef OPTIONS_H
#define	OPTIONS_H

#include "defs.h"

#include <string>
#include <vector>

namespace pm {
    
    ////////////////////////////////////////////////////////////////////////////
    ///// Simple types /////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    template <typename Scalar>
    inline mxArray *mxCreateScalar(Scalar s) {
        mxArray *d = mxCreateNumericArray(1, 1, classID<Scalar>());
        Scalar *ptr = reinterpret_cast<Scalar *>(mxGetData(d));
        *ptr = s;
        return d;
    }

    inline double mxCheckedScalar(const mxArray *a, const char *s) {
        if (!mxIsNumeric(a)) {
            std::cerr << "ClassID = " << mxGetClassID(a) << "\n";
            mexErrMsgIdAndTxt("MATLAB:mex:invalidInput", s);
        }
        return mxGetScalar(a);
    }

    template <typename Scalar>
    inline void mxLoadScalars(Scalar *ptr, int n, const mxArray *arr, const char *s) {
        if (!mxIsNumeric(arr)) {
            mexErrMsgIdAndTxt("MATLAB:mex:invalidInput", s);
        }
        if (mxGetClassID(arr) != classID<Scalar>()) {
            mexErrMsgIdAndTxt("MATLAB:mex:invalidInput", s);
        }
        const Scalar *data = reinterpret_cast<const Scalar *> (mxGetData(arr));
        switch (mxGetNumberOfElements(arr)) {
            case 1:
                std::fill(ptr, ptr + n, Scalar(mxGetScalar(arr)));
                break;
            case 5:
                for (int i = 0; i < 5; ++i) ptr[i] = data[i];
                break;
            default:
                mexErrMsgIdAndTxt("MATLAB:mex:invalidInput", s);
                break;
        }
    }

    template <typename S, typename T>
    inline void mxLoadVector(std::vector<T> &v, const mxArray *arr, const char *s) {
        if (!mxIsNumeric(arr)) {
            mexErrMsgIdAndTxt("MATLAB:mex:invalidInput", s);
        }
        const S *data = reinterpret_cast<const S *> (mxGetData(arr));
        int N = mxGetNumberOfElements(arr);
        if(N == 0) mexErrMsgIdAndTxt("MATLAB:mex:mxLoadVector", s);
        // resize the vector and fill it
        v.resize(N);
        for(int i = 0; i < N; ++i) v[i] = T(data[i]);
    }

    template <typename T>
    inline void mxLoadVector(std::vector<T> &v, const mxArray *arr, const char *s) {
        switch (mxGetClassID(arr)) {
            case mxINT8_CLASS: mxLoadVector<char, T>(v, arr, s); break;
            case mxUINT8_CLASS: mxLoadVector<unsigned char, T>(v, arr, s); break;
            case mxSINGLE_CLASS: mxLoadVector<float, T>(v, arr, s); break;
            case mxDOUBLE_CLASS: mxLoadVector<double, T>(v, arr, s); break;
            default:
                mexErrMsgIdAndTxt("MATLAB:mex:mxLoadVector", "Class type not supported!");
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////
    ///// Field members ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    inline bool mxHasField(const mxArray *options, mwIndex i, const char *fieldname) {
        if(!mxIsStruct(options)){
            mexErrMsgIdAndTxt("MATLAB:mex:mxHasField", "Invalid call mxHasField for '%s' on non-struct object.", fieldname);
            return false;
        }
        return mxGetField(options, i, fieldname) != NULL;
    }

    template <typename T = double>
    inline T mxScalarField(const mxArray *options, mwIndex i, const char *fieldname, T defaultValue = 0) {
        if(!mxIsStruct(options)){
            mexErrMsgIdAndTxt("MATLAB:mex:mxScalarField", "Invalid call mxScalarField for '%s' on non-struct object.", fieldname);
            return defaultValue;
        }
        const mxArray *tmp = mxGetField(options, i, fieldname);
        if(tmp == NULL)
            return defaultValue;
        if(!mxIsNumeric(tmp))
            return defaultValue;
        return mxGetScalar(tmp);
    }

    inline bool mxBoolField(const mxArray *options, mwIndex i, const char *fieldname, bool defValue = false) {
        if(!mxIsStruct(options)){
            mexErrMsgIdAndTxt("MATLAB:mex:mxBoolField", "Invalid call mxBoolField for '%s' on non-struct object.", fieldname);
            return defValue;
        }
        const mxArray *tmp = mxGetField(options, i, fieldname);
        if(!tmp) return defValue;
        if(mxIsNumeric(tmp)){
            return mxGetScalar(tmp) != 0;
        } else if(mxIsLogical(tmp) && mxGetNumberOfElements(tmp) > 0) {
            return mxGetLogicals(tmp)[0];
        } else {
            mexErrMsgIdAndTxt("MATLAB:mex:invalidInput", "Parameter %s is not a valid boolean expression.", fieldname);
            return defValue;
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////
    ///// Option data-structure ////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    template <typename T>
    class Parameter {
    public:
        explicit Parameter(const std::string &name)
        : label(name), valid(false) {
        }
        explicit Parameter(const std::string &name, T def)
        : label(name), value(def), defaultValue(def), valid(true) {
        }
        
        bool load(const mxArray *options) {
            if(mxHasField(options, label.c_str())){
                
            }
        }
        
        bool empty() const {
            return !valid;
        }
        
    private:
        std::string label;
        T value;
        T defaultValue;
        bool valid;
    };
}

#endif	/* OPTIONS_H */

