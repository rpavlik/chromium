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
	CRbitvalue enable[CR_MAX_BITARRAY];
	CRbitvalue width[CR_MAX_BITARRAY];
	CRbitvalue stipple[CR_MAX_BITARRAY];
	CRbitvalue dirty[CR_MAX_BITARRAY];
} CRLineBits;

typedef struct {
	GLboolean	lineSmooth;
	GLboolean	lineStipple;
	GLfloat		width;
	GLushort	pattern;
	GLint		repeat;
} CRLineState;

void crStateLineInit (CRContext *ctx);

void crStateLineDiff(CRLineBits *bb, CRbitvalue *bitID, 
		CRLineState *from, CRLineState *to);
void crStateLineSwitch(CRLineBits *bb, CRbitvalue *bitID, 
		CRLineState *from, CRLineState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_LINE_H */
