#ifndef CR_STATE_EVALUATORS
#define CR_STATE_EVALUATORS

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#define GLEVAL_TOT 9

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue eval1d[GLEVAL_TOT];
	GLbitvalue eval2d[GLEVAL_TOT];
	GLbitvalue enable;
	GLbitvalue enable1d[GLEVAL_TOT];
	GLbitvalue enable2d[GLEVAL_TOT];
	GLbitvalue grid1d;
	GLbitvalue grid2d;
	GLbitvalue dirty;
} CREvaluatorBits;

typedef struct {
	GLdouble  u1, u2;
	GLint    order;
	GLdouble  *coeff;
} CREvaluator1d;

typedef struct {
	GLdouble  u1, u2;
	GLdouble  v1, v2;
	GLint    uorder;
	GLint    vorder;
	GLdouble  *coeff;
} CREvaluator2d;

typedef struct {
	GLboolean  enable1d[GLEVAL_TOT];
	GLboolean  enable2d[GLEVAL_TOT];
	GLboolean  autoNormal;

	GLint      maxEvalOrder;

	CREvaluator1d   eval1d[GLEVAL_TOT];
	CREvaluator2d   eval2d[GLEVAL_TOT];

	GLint      un1d;
	GLdouble   u11d, u21d;

	GLint      un2d;
	GLint      vn2d;
	GLdouble   u12d, u22d;
	GLdouble   v12d, v22d;
} CREvaluatorState;

extern const int gleval_sizes[];

void crStateEvaluatorInitBits (CREvaluatorBits *);
void crStateEvaluatorInit (CREvaluatorState *e);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_EVALUATORS */
