/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_EVALUATORS
#define CR_STATE_EVALUATORS

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#define GLEVAL_TOT 9

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue eval1D[GLEVAL_TOT];
	GLbitvalue eval2D[GLEVAL_TOT];
	GLbitvalue enable;
	GLbitvalue enable1D[GLEVAL_TOT];
	GLbitvalue enable2D[GLEVAL_TOT];
	GLbitvalue grid1D;
	GLbitvalue grid2D;
	GLbitvalue dirty;
} CREvaluatorBits;

typedef struct {
	GLdouble  u1, u2;
	GLint    order;
	GLdouble  *coeff;
} CREvaluator1D;

typedef struct {
	GLdouble  u1, u2;
	GLdouble  v1, v2;
	GLint    uorder;
	GLint    vorder;
	GLdouble  *coeff;
} CREvaluator2D;

typedef struct {
	GLboolean  enable1D[GLEVAL_TOT];
	GLboolean  enable2D[GLEVAL_TOT];
	GLboolean  autoNormal;

	GLint      maxEvalOrder;

	CREvaluator1D   eval1D[GLEVAL_TOT];
	CREvaluator2D   eval2D[GLEVAL_TOT];

	GLint      un1D;
	GLdouble   u11D, u21D;

	GLint      un2D;
	GLint      vn2D;
	GLdouble   u12D, u22D;
	GLdouble   v12D, v22D;
} CREvaluatorState;

extern const int gleval_sizes[];

void crStateEvaluatorInitBits (CREvaluatorBits *);
void crStateEvaluatorInit (CREvaluatorState *e);

void crStateEvaluatorDiff(CREvaluatorBits *bb, GLbitvalue bitID, 
		CREvaluatorState *from, CREvaluatorState *to);
void crStateEvaluatorSwitch(CREvaluatorBits *bb, GLbitvalue bitID, 
		CREvaluatorState *from, CREvaluatorState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_EVALUATORS */
