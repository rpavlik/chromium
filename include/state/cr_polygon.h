/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_POLYGON_H
#define CR_STATE_POLYGON_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue enable[CR_MAX_BITARRAY];
	GLbitvalue offset[CR_MAX_BITARRAY];
	GLbitvalue mode[CR_MAX_BITARRAY];
	GLbitvalue stipple[CR_MAX_BITARRAY];
	GLbitvalue dirty[CR_MAX_BITARRAY];
} CRPolygonBits;

typedef struct {
	GLboolean	polygonSmooth;
	GLboolean polygonOffsetFill;
	GLboolean polygonOffsetLine;
	GLboolean polygonOffsetPoint;
	GLboolean	polygonStipple;
	GLboolean cullFace;
	GLfloat		offsetFactor;
	GLfloat		offsetUnits;
	GLenum		cullFaceMode;
	GLenum		frontFace;
	GLenum		frontMode;
	GLenum		backMode;
	GLint		  stipple[32];
} CRPolygonState;

void crStatePolygonInitBits(CRPolygonBits *p);
void crStatePolygonInit(CRPolygonState *p);

void crStatePolygonDiff(CRPolygonBits *bb, GLbitvalue *bitID, 
		CRPolygonState *from, CRPolygonState *to);
void crStatePolygonSwitch(CRPolygonBits *bb, GLbitvalue *bitID, 
		CRPolygonState *from, CRPolygonState *to);

#ifdef __cplusplus
}
#endif

#endif
