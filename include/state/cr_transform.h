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
	GLbitvalue dirty[CR_MAX_BITARRAY];
	GLbitvalue mode[CR_MAX_BITARRAY];
	GLbitvalue matrix[4][CR_MAX_BITARRAY];
	GLbitvalue clipPlane[CR_MAX_BITARRAY];
	GLbitvalue enable[CR_MAX_BITARRAY];
	GLbitvalue base[CR_MAX_BITARRAY];
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

	GLmatrix  *modelView;
	GLmatrix  *projection;
	GLmatrix  *texture[CR_MAX_TEXTURE_UNITS];
	GLmatrix  *color;
	GLmatrix  *m;

	GLboolean  transformValid;
	GLmatrix  transform;
#ifdef CR_OPENGL_VERSION_1_2
	GLboolean rescaleNormals;
#endif
	GLboolean normalize;
} CRTransformState;

void crStateTransformInitBits (CRTransformBits *tb);
void crStateTransformSetOutputBounds (CRTransformState *t,
		GLrecti *outputwindow,
		GLrecti *imagespace,
		GLrecti *imagewindow);

void crStateTransformInit (CRLimitsState *limits, CRTransformState *t);
void crStateTransformUpdateTransform(CRTransformState *t);
void crStateTransformXformPoint  (CRTransformState *t, GLvectorf *p);
void crStateTransformXformPointMatrixf(GLmatrix *m, GLvectorf *p);
void crStateTransformXformPointMatrixd(GLmatrix *m, GLvectord *p);
void crStateTransformInvertTransposeMatrix(GLmatrix *inv, GLmatrix *mat);
void crStateTransformNormalizedDims(GLrectf *p, GLrecti *r, GLrecti *q);

void crStateTransformDiff(GLuint maxTextureUnits,
		CRTransformBits *bb, GLbitvalue *bitID, 
		CRTransformState *from, CRTransformState *to);
void crStateTransformSwitch(GLuint maxTextureUnits, 
		CRTransformBits *bb, GLbitvalue *bitID, 
		CRTransformState *from, CRTransformState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TRANSFORM_H */
