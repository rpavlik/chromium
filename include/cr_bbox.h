/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_BBOX_H
#define CR_BBOX_H

#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void crTransformBBox( float xmin, float ymin, float zmin, float xmax, float ymax, float zmax, GLmatrix *m, float *out_xmin, float *out_ymin, float *out_zmin, float *out_xmax, float *out_ymax, float *out_zmax );

#ifdef __cplusplus
}
#endif

#endif /* CR_BBOX_H */
