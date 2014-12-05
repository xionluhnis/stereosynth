/* 
 * File:   images.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 17, 2014, 5:16 PM
 */

#ifndef IMAGES_H
#define	IMAGES_H

#include "defs.h"
#include "../math/mat.h"
#include "../math/imageset.h"

namespace pm {
    
    inline mxArray *mxCreateMatrix(int rows, int cols, mxClassID type = mxSINGLE_CLASS) {
        mwSize sz[2] = {rows, cols};
        return mxCreateNumericArray(2, sz, type, mxREAL);
    }

    inline mxArray *mxCreateMatrix(int rows, int cols, int channels, mxClassID type = mxSINGLE_CLASS) {
        mwSize sz[3] = {rows, cols, channels};
        return mxCreateNumericArray(3, sz, type, mxREAL);
    }
    
    template< typename T >
    inline mxArray *mxCreateMatrix(int rows, int cols, int channels){
        return mxCreateMatrix(rows, cols, channels, classID<T>());
    }

    template <typename Scalar>
    inline mxArray *mxCreateMatrix(const Image &img) {
        return mxCreateMatrix(img.rows, img.cols, img.channels(), classID<Scalar>());
    }

    inline mxArray *mxCreateMatrix(const Image &img) {
        switch (img.depth()) {
            case IM_8U: return mxCreateMatrix(img.rows, img.cols, img.channels(), mxUINT8_CLASS);
            case IM_32F: return mxCreateMatrix(img.rows, img.cols, img.channels(), mxSINGLE_CLASS);
            case IM_64F: return mxCreateMatrix(img.rows, img.cols, img.channels(), mxDOUBLE_CLASS);
            default:
                mexErrMsgIdAndTxt("MATLAB:mex:createMatrixFor", "Class type not supported!");
                return NULL;
        }
    }
    
    template <typename Scalar>
    inline mxArray *mxImageToArray(const Image &img) {
        // std::cout << "mxImageToArray\n";
        mxArray *arr = mxCreateMatrix<Scalar>(img);
        const int offset = img.rows * img.cols;
        Scalar *data = reinterpret_cast<Scalar *> (mxGetData(arr));
        // #pragma omp parallel for collapse(2)
        for (int y = 0; y < img.rows; ++y) {
            for (int x = 0; x < img.cols; ++x) {
                const Scalar *iptr = img.ptr<Scalar>(y, x);
                for (int ch = 0; ch < img.channels(); ++ch) {
                    // transposing!
                    data[y + x * img.rows + offset * ch] = iptr[ch];
                }
            }
        }
        return arr;
        // return MxArray(img, mxUNKNOWN_CLASS, false);
    }

    inline mxArray *mxImageToArray(const Image &img) {
        switch (img.depth()) {
            case IM_8S: return mxImageToArray<char>(img);
            case IM_8U: return mxImageToArray<unsigned char>(img);
            case IM_32F: return mxImageToArray<float>(img);
            case IM_64F: return mxImageToArray<double>(img);
            default:
                mexErrMsgIdAndTxt("MATLAB:mex:mxImageToArray", "Class type not supported!");
                return NULL;
        }
    }

    template <typename Scalar>
    inline void mxCheckImage(const Image &img, const char *errMsg = "Corrupted image with pixels out of bounds!") {
        bool err = false;
        for (int y = 0; y < img.rows; ++y) {
            for (int x = 0; x < img.cols; ++x) {
                const Scalar *ptr = img.ptr<Scalar>(y, x);
                for (int c = 0; c < img.channels(); ++c) {
                    Scalar d = ptr[c];
                    if ((d + 1) == d) {
                        std::cout << "Invalid p@" << y << "/" << x << "/c" << c << " = " << d << "\n";
                        err = true;
                    }
                }
            }
        }
        if (err) mexErrMsgIdAndTxt("MATLAB:mex:invalid_image", errMsg);
    }

    template <typename Scalar>
    inline Image mxArrayToImage(const mxArray *arr, const char *errMsg) {
        if (!mxIsNumeric(arr)) {
            mexErrMsgIdAndTxt("MATLAB:mex:invalidInput", "Invalid image array.");
        }
        int h = mxGetDimensions(arr)[0];
        int w = mxGetDimensions(arr)[1];
        int num_ch = mxGetNumberOfDimensions(arr) < 3 ? 1 : mxGetDimensions(arr)[2];
        const int offset = h * w; 
        
        assert(mxGetClassID(arr) == classID<Scalar>());
        
        Image img(h, w, IM_MAKETYPE(DataDepth<Scalar>::value, num_ch));
        const Scalar *data = reinterpret_cast<const Scalar *> (mxGetData(arr));
        
        // /!\ img.step[2] might be wrong! do not use at(y, x, ch) indexing!
        // => use img.ptr(y, x)
        if (num_ch == 1) {
            for (int y = 0; y < img.rows; ++y) {
                for (int x = 0; x < img.cols; ++x) {
                    // transposing!
                    img.at<Scalar>(y, x) = data[y + x * img.rows];
                }
            }
        } else {
            for (int y = 0; y < img.rows; ++y) {
                for (int x = 0; x < img.cols; ++x) {
                    Scalar *iptr = img.ptr<Scalar>(y, x);
                    for (int ch = 0; ch < img.channels(); ++ch) {
                        // std::cout << "at(" << y << ", " << x << ", " << ch << ") = ";
                        // transposing!
                        Scalar v = data[y + x * img.rows + offset * ch];
                        // std::cout << v << "\n";
                        iptr[ch] = v;
                    }
                }
            }
        }
        mxCheckImage<Scalar>(img, errMsg);
        return img;
    }

    inline Image mxArrayToImage(const mxArray *arr, const char *err = "Corrupted image with pixels out of bounds!") {
        switch (mxGetClassID(arr)) {
            case mxINT8_CLASS: return mxArrayToImage<char>(arr, err);
            case mxUINT8_CLASS: return mxArrayToImage<unsigned char>(arr, err);
            case mxSINGLE_CLASS: return mxArrayToImage<float>(arr, err);
            case mxDOUBLE_CLASS: return mxArrayToImage<double>(arr, err);
            default:
                mexErrMsgIdAndTxt("MATLAB:mex:mxArrayToImage", "Class type not supported!");
                return Image();
        }
    }
    
    inline ImageSet mxArrayToImageSet(const mxArray *arr, const char *err = "Invalid image set") {
        if(!mxIsCell(arr)){
            mexErrMsgIdAndTxt("MATLAB:mex:mxArrayToImageSet", "Image set should be of cell type.");
        }
        size_t N = mxGetNumberOfElements(arr);
        ImageSet set(N);
        for(unsigned int i = 0; i < N; ++i){
            const mxArray *cell = mxGetCell(arr, i);
            if(cell){
                set[i] = mxArrayToImage(cell, "Invalid cell image for image set");
            } else {
                mexErrMsgIdAndTxt("MATLAB:mex:mxArrayToImageSet", "Cell image was empty for image set!");
            }
        }
        return set;
    }
    
}

#endif	/* IMAGES_H */

