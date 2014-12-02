/* 
 * File:   strings.h
 * Author: Alexandre Kaspar <akaspar@mit.edu>
 *
 * Created on November 17, 2014, 5:12 PM
 */

#ifndef STRINGS_H
#define	STRINGS_H

#include "defs.h"

#include <cstring>

namespace pm {
    
    inline bool mxStringEquals(const mxArray *A, const char *s) {
        char buf[256];
        if (!mxIsChar(A)) {
            return false;
        }
        if (mxGetString(A, buf, 255)) {
            return false;
        }
        return strcmp(s, buf) == 0;
    }
}

#endif	/* STRINGS_H */

