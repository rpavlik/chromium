/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef GLTRANS_H
#define GLTRANS_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue dirty;
	GLbitvalue mode;
	GLbitvalue matrix[4];
	GLbitvalue clipPlane;
	GLbitvalue enable;
	GLbitvalue base;
} CRTransformBits;

typedef struct {
	GLenum    mode;
	GLint    matrixid;

	GLint    maxModelViewStackDepth;
	GLint    maxProjectionStackDepth;
	GLint    maxTextureStackDepth;
	GLint    maxColorStackDepth;
	GLint    maxDepth;

	GLint    modelViewDepth;
	GLint    projectionDepth;
	GLint    textureDepth[CR_MAX_TEXTURE_UNITS];
	GLint    colorDepth;
	GLint    *depth;

	GLint    maxClipPlanes;
	GLvectord  *clipPlane;
	GLboolean  *clip;

	GLmatrix  *modelView;
	GLmatrix  *projection;
	GLmatrix  *texture[CR_MAX_TEXTURE_UNITS];
	GLmatrix  *color;
	GLmatrix  *m;

	GLboolean  transformValid;
	GLmatrix  transform;
} CRTransformState;

void crStateTransformInitBits (CRTransformBits *tb);
void crStateTransformSetOutputBounds (CRTransformState *t,
		GLrecti *outputwindow,
		GLrecti *imagespace,
		GLrecti *imagewindow);

void crStateTransformInit (CRTransformState *);
void crStateTransformUpdateTransform(CRTransformState *t);
void crStateTransformXformPoint  (CRTransformState *t, GLvectorf *p);
void crStateTransformXformPointMatrixf(GLmatrix *m, GLvectorf *p);
void crStateTransformXformPointMatrixd(GLmatrix *m, GLvectord *p);
void crStateTransformInvertTransposeMatrix(GLmatrix *inv, GLmatrix *mat);
void crStateTransformNormalizedDims(GLrectf *p, GLrecti *r, GLrecti *q);

void crStateTransformDiff(CRTransformBits *bb, GLbitvalue bitID, 
		CRTransformState *from, CRTransformState *to);
void crStateTransformSwitch(CRTransformBits *bb, GLbitvalue bitID, 
		CRTransformState *from, CRTransformState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TRANSFORM_H */
