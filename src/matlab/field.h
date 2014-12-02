/* 
 * File:   field.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 18, 2014, 11:29 AM
 */

#ifndef MATLAB_FIELD_H
#define	MATLAB_FIELD_H

#include "defs.h"
#include "wrapper.h"
#include "../nnf/field.h"

namespace pm {
    
    template <int RowMajor>
    void mxLoadField(Field2D<RowMajor> *field, const mxArray *arr, const char *errMsg) {
        MatXD data(arr);
        
        for(Mat &m : field->layers()){
            
        }
    }
    
    template <typename T = float, int RowMajor = true>
    mxArray *mxSaveField(const Field2D<RowMajor> *field, const char *errMsg) {
        int channels = field->totalChannels<T>();
        // create matrix data
        mxArray *m = mxCreateMatrix<T>(field->rows, field->cols, channels);
        
        // fill matrix data using a wrapper
        MatXD mat(m);
        int ch = 0;
        for(const auto &layer : field->layers()){
            auto layout = field->layout<T>(layer);
            layout
            channels = layer.elemSize() / bytesPerChannel;
            int extraBytes = layer.elemSize() % bytesPerChannel;
            // temporary storage
            std::vector<T> storage(channels);
            T extra[sizeof(T)] = { 0 };
            // copy data
            for(const Point2i &p : layer){
                // load
                
                // save
                
            }
        }
        
        return m;
    }
    
}

#endif	/* MATLAB_FIELD_H */

