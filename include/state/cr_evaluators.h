/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_EVALUATORS
#define CR_STATE_EVALUATORS

#include "state/cr_statetypes.h"

#define GLEVAL_TOT 9
#define MAX_EVAL_ORDER 30

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CRbitvalue eval1D[GLEVAL_TOT][CR_MAX_BITARRAY];
	CRbitvalue eval2D[GLEVAL_TOT][CR_MAX_BITARRAY];
	CRbitvalue enable[CR_MAX_BITARRAY];
	CRbitvalue enable1D[GLEVAL_TOT][CR_MAX_BITARRAY];
	CRbitvalue enable2D[GLEVAL_TOT][CR_MAX_BITARRAY];
	CRbitvalue grid1D[CR_MAX_BITARRAY];
	CRbitvalue grid2D[CR_MAX_BITARRAY];
	CRbitvalue dirty[CR_MAX_BITARRAY];
} CREvaluatorBits;

typedef struct {
	GLfloat  u1, u2;
	GLfloat  du;
	GLint    order;
	GLfloat  *coeff;
} CREvaluator1D;

typedef struct {
	GLfloat  u1, u2;
	GLfloat  v1, v2;
	GLfloat  du, dv;
	GLint    uorder;
	GLint    vorder;
	GLfloat  *coeff;
} CREvaluator2D;

typedef struct {
	GLboolean  enable1D[GLEVAL_TOT];
	GLboolean  enable2D[GLEVAL_TOT];
	GLboolean  autoNormal;

	CREvaluator1D   eval1D[GLEVAL_TOT];
	CREvaluator2D   eval2D[GLEVAL_TOT];

	GLint      un1D;        /* GL_MAP1_GRID_SEGMENTS */
	GLfloat    u11D, u21D;  /* GL_MAP1_GRID_DOMAIN */

	GLint      un2D;        /* GL_MAP2_GRID_SEGMENTS (u) */
	GLint      vn2D;        /* GL_MAP2_GRID_SEGMENTS (v) */
	GLfloat    u12D, u22D;  /* GL_MAP2_GRID_DOMAIN (u) */
	GLfloat    v12D, v22D;  /* GL_MAP2_GRID_DOMAIN (v) */
} CREvaluatorState;

extern const int gleval_sizes[];

void crStateEvaluatorInit (CRContext *ctx);
void crStateEvaluatorDestroy (CRContext *ctx);

void crStateEvaluatorDiff(CREvaluatorBits *bb, CRbitvalue *bitID, 
		CREvaluatorState *from, CREvaluatorState *to);
void crStateEvaluatorSwitch(CREvaluatorBits *bb, CRbitvalue *bitID, 
		CREvaluatorState *from, CREvaluatorState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_EVALUATORS */
