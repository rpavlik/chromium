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
	CRbitvalue *currentMatrix; /* points to one of the following */
	CRbitvalue matrixMode[CR_MAX_BITARRAY];
	CRbitvalue modelviewMatrix[CR_MAX_BITARRAY];
	CRbitvalue projectionMatrix[CR_MAX_BITARRAY];
	CRbitvalue colorMatrix[CR_MAX_BITARRAY];
	CRbitvalue textureMatrix[CR_MAX_BITARRAY];
	CRbitvalue programMatrix[CR_MAX_BITARRAY];
	CRbitvalue clipPlane[CR_MAX_BITARRAY];
	CRbitvalue enable[CR_MAX_BITARRAY];
	CRbitvalue base[CR_MAX_BITARRAY];
} CRTransformBits;

typedef struct {
   CRmatrix *top;      /* points into stack */
   CRmatrix *stack;    /* array [maxDepth] of CRmatrix */
   GLuint depth;       /* 0 <= depth < maxDepth */
   GLuint maxDepth;    /* size of stack[] array */
} CRMatrixStack;

typedef struct {
	GLvectord  *clipPlane;
	GLboolean  *clip;

	GLenum matrixMode;

	/* matrix stacks */
	CRMatrixStack modelViewStack;
	CRMatrixStack projectionStack;
	CRMatrixStack colorStack;
	CRMatrixStack textureStack[CR_MAX_TEXTURE_UNITS];
	CRMatrixStack programStack[CR_MAX_PROGRAM_MATRICES];
	CRMatrixStack *currentStack;

	GLboolean modelViewProjectionValid;
	CRmatrix  modelViewProjection;  /* product of modelview and projection */

#ifdef CR_OPENGL_VERSION_1_2
	GLboolean rescaleNormals;
#endif
#ifdef CR_IBM_rasterpos_clip
	GLboolean rasterPositionUnclipped;
#endif
	GLboolean normalize;
} CRTransformState;


void crStateTransformInit(CRContext *ctx);
void crStateTransformDestroy(CRContext *ctx);

void crStateTransformUpdateTransform(CRTransformState *t);
void crStateTransformXformPoint(CRTransformState *t, GLvectorf *p);

void crStateTransformXformPointMatrixf(const CRmatrix *m, GLvectorf *p);
void crStateTransformXformPointMatrixd(const CRmatrix *m, GLvectord *p);
void crStateTransformInvertTransposeMatrix(CRmatrix *inv, const CRmatrix *mat);

void crStateTransformDiff(CRTransformBits *t, CRbitvalue *bitID,
                          CRContext *fromCtx, CRContext *toCtx);

void crStateTransformSwitch(CRTransformBits *bb, CRbitvalue *bitID, 
                            CRContext *fromCtx, CRContext *toCtx);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TRANSFORM_H */
