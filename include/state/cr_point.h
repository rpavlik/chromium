/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_LINE_H
#define SR_STATE_LINE_H

#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue enable[CR_MAX_BITARRAY];
	GLbitvalue size[CR_MAX_BITARRAY];
	GLbitvalue dirty[CR_MAX_BITARRAY];
} CRPointBits;

typedef struct {
	GLboolean	pointSmooth;
	GLfloat		pointSize;
} CRPointState;

void crStatePointInitBits (CRPointBits *l);
void crStatePointInit (CRPointState *l);

void crStatePointDiff(CRPointBits *bb, GLbitvalue *bitID, 
		CRPointState *from, CRPointState *to);
void crStatePointSwitch(CRPointBits *bb, GLbitvalue *bitID, 
		CRPointState *from, CRPointState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_LINE_H */
