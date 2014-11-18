/* 
 * File:   field.h
 * Author: akaspar
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
    
    template <int RowMajor>
    mxArraz *mxSaveField(const Field2D<RowMajor> *field, const char *errMsg) {
        
        return NULL;
    }
    
}

#endif	/* MATLAB_FIELD_H */

