/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_LINE_H
#define SR_STATE_LINE_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue enable;
	GLbitvalue size;
	GLbitvalue width;
	GLbitvalue stipple;
	GLbitvalue dirty;
} CRLineBits;

typedef struct {
	GLboolean	lineSmooth;
	GLboolean	lineStipple;
	GLboolean	pointSmooth;

	GLfloat		pointSize;
	GLfloat		width;
	GLushort	pattern;
	GLint		repeat;
} CRLineState;

void crStateLineInitBits (CRLineBits *l);
void crStateLineInit (CRLineState *l);

void crStateLineDiff(CRLineBits *bb, GLbitvalue bitID, 
		CRLineState *from, CRLineState *to);
void crStateLineSwitch(CRLineBits *bb, GLbitvalue bitID, 
		CRLineState *from, CRLineState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_LINE_H */
