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

#define NUM_MATRICES 4

typedef struct {
	CRbitvalue dirty[CR_MAX_BITARRAY];
	CRbitvalue matrixMode[CR_MAX_BITARRAY];
	CRbitvalue matrix[NUM_MATRICES][CR_MAX_BITARRAY];
	CRbitvalue clipPlane[CR_MAX_BITARRAY];
	CRbitvalue enable[CR_MAX_BITARRAY];
	CRbitvalue base[CR_MAX_BITARRAY];
} CRTransformBits;

typedef struct {
	GLenum   matrixMode;
	GLint    matrixid;    /* 0=modelview, 1=projection, 2=texture, 3=color */

	GLint    maxDepth;		/* Max depth of current matrix stack/mode */

	GLint    modelViewDepth;
	GLint    projectionDepth;
	GLint    textureDepth[CR_MAX_TEXTURE_UNITS];
	GLint    colorDepth;
	GLint    *depth;      /* points to one of above depth fields */

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
#ifdef CR_IBM_rasterpos_clip
	GLboolean rasterPositionUnclipped;
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

void crStateTransformDiff(CRTransformBits *t, CRbitvalue *bitID,
                          CRContext *fromCtx, CRContext *toCtx);

void crStateTransformSwitch(CRTransformBits *bb, CRbitvalue *bitID, 
                            CRContext *fromCtx, CRContext *toCtx);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TRANSFORM_H */
