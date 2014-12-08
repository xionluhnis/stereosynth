/* 
 * File:   arguments.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 17, 2014, 5:14 PM
 */

#ifndef OPTIONS_H
#define	OPTIONS_H

#include "defs.h"
#include "images.h"

#include <string>
#include <vector>

namespace pm {
    
    ////////////////////////////////////////////////////////////////////////////
    ///// Simple types /////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    template <typename Scalar>
    inline mxArray *mxCreateNothing() {
        mwSize sz[2] = {0, 0};
        mxArray *d =  mxCreateNumericArray(2, sz, classID<Scalar>(), mxREAL);
        return d;
    }
    inline mxArray *mxCreateNothing() {
        return mxCreateNothing<float>();
    }
    
    template <typename Scalar>
    inline mxArray *mxCreateScalar(Scalar s) {
        mwSize sz[2] = {1, 1};
        mxArray *d =  mxCreateNumericArray(2, sz, classID<Scalar>(), mxREAL);
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

    template <typename T, typename S>
    inline void mxLoadVector(std::vector<T> *v_ptr, const mxArray *arr, const char *s) {
        if (!mxIsNumeric(arr)) {
            mexErrMsgIdAndTxt("MATLAB:mex:invalidInput", s);
        }
        const S *data = reinterpret_cast<const S *> (mxGetData(arr));
        int N = mxGetNumberOfElements(arr);
        if(N == 0) mexErrMsgIdAndTxt("MATLAB:mex:mxLoadVector", s);
        
        // vector reference
        std::vector<T> &v = *v_ptr;
        // resize the vector and fill it
        v.resize(N);
        for(int i = 0; i < N; ++i) v[i] = T(data[i]);
    }

    template <typename T>
    inline void mxLoadVector(std::vector<T> *v, const mxArray *arr, const char *s) {
        switch (mxGetClassID(arr)) {
            case mxINT8_CLASS: mxLoadVector<T, char>(v, arr, s); break;
            case mxUINT8_CLASS: mxLoadVector<T, unsigned char>(v, arr, s); break;
            case mxINT32_CLASS: mxLoadVector<T, int>(v, arr, s); break;
            case mxUINT32_CLASS: mxLoadVector<T, unsigned int>(v, arr, s); break;
            case mxSINGLE_CLASS: mxLoadVector<T, float>(v, arr, s); break;
            case mxDOUBLE_CLASS: mxLoadVector<T, double>(v, arr, s); break;
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
    
    class mxOptions {
    public:
        
        typedef const char* FieldName;
        
        mxOptions(const mxArray *arr) : options(arr){
            if(!mxIsStruct(arr) && mxGetNumberOfElements(arr) > 0){
                mexErrMsgIdAndTxt("MATLAB:mex:options", "Invalid mxOptions on non-struct data.");
            }
        }
        
        bool has(FieldName name) const {
            return mxHasField(options, 0, name);
        }
        
        template <typename S = double>
        S scalar(FieldName name, S defaultValue) const {
            if(const mxArray *field = mxGetField(options, 0, name)){
                return mxCheckedScalar(field, name);
            }
            return defaultValue;
        }
        
        int integer(FieldName name, int defaultValue) const {
            return scalar<int>(name, defaultValue);
        }
        
        bool boolean(FieldName name, bool defaultValue) const {
            return scalar<bool>(name, defaultValue);
        }
        
        template <typename S = double>
        std::vector<S> vector(FieldName name) const {
            std::vector<S> v;
            if(const mxArray *field = mxGetField(options, 0, name)){
                mxLoadVector<S>(&v, field, name);
            }
            return v;
        }
        
        template <typename S = double>
        std::vector<S> vector(FieldName name, S defaultValue, int minSize) const {
            std::vector<S> v = vector<S>(name);
            if(v.size() >= minSize) return v;
            // single element => fill with it
            if(v.size() == 1) {
                defaultValue = v[0];
            }
            v.resize(minSize);
            std::fill(v.begin(), v.end(), defaultValue);
            return v;
        }
        
        Image image(FieldName name) const {
            Image img;
            if(const mxArray *field = mxGetField(options, 0, name)){
                img = mxArrayToImage(field, name);
            }
            return img;
        }
        
    private:
        const mxArray *options;
    };
}

#endif	/* OPTIONS_H */

