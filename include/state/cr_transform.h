/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef GLTRANS_H
#define GLTRANS_H

#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CRbitvalue dirty[CR_MAX_BITARRAY];
	CRbitvalue mode[CR_MAX_BITARRAY];
	CRbitvalue matrix[4][CR_MAX_BITARRAY];
	CRbitvalue clipPlane[CR_MAX_BITARRAY];
	CRbitvalue enable[CR_MAX_BITARRAY];
	CRbitvalue base[CR_MAX_BITARRAY];
} CRTransformBits;

typedef struct {
	GLenum    mode;
	GLint    matrixid;

	GLint    maxDepth;		/* Max depth of current matrix stack/mode */

	GLint    modelViewDepth;
	GLint    projectionDepth;
	GLint    textureDepth[CR_MAX_TEXTURE_UNITS];
	GLint    colorDepth;
	GLint    *depth;

	GLvectord  *clipPlane;
	GLboolean  *clip;

	CRmatrix  *modelView;
	CRmatrix  *projection;
	CRmatrix  *texture[CR_MAX_TEXTURE_UNITS];
	CRmatrix  *color;
	CRmatrix  *m;

	GLboolean  transformValid;
	CRmatrix  transform;
#ifdef CR_OPENGL_VERSION_1_2
	GLboolean rescaleNormals;
#endif
	GLboolean normalize;
} CRTransformState;

void crStateTransformSetOutputBounds (CRTransformState *t,
		CRrecti *outputwindow,
		CRrecti *imagespace,
		CRrecti *imagewindow);

void crStateTransformInit (CRContext *ctx);
void crStateTransformDestroy (CRContext *ctx);

void crStateTransformUpdateTransform(CRTransformState *t);
void crStateTransformXformPoint  (CRTransformState *t, GLvectorf *p);
void crStateTransformXformPointMatrixf(CRmatrix *m, GLvectorf *p);
void crStateTransformXformPointMatrixd(CRmatrix *m, GLvectord *p);
void crStateTransformInvertTransposeMatrix(CRmatrix *inv, CRmatrix *mat);
void crStateTransformNormalizedDims(CRrectf *p, CRrecti *r, CRrecti *q);

void crStateTransformDiff(GLuint maxTextureUnits,
		CRTransformBits *bb, CRbitvalue *bitID, 
		CRTransformState *from, CRTransformState *to);
void crStateTransformSwitch(GLuint maxTextureUnits, 
		CRTransformBits *bb, CRbitvalue *bitID, 
		CRTransformState *from, CRTransformState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TRANSFORM_H */
