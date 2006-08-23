
#ifndef CR_MATRIX_H
#define CR_MATRIX_H

#include "chromium.h"

/*
 * Note: m[col][row] matches OpenGL's column-major memory layout
 */
typedef struct {
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;
} CRmatrix;

typedef struct { GLfloat x,y,z,w; } GLvectorf;
typedef struct { GLdouble x,y,z,w; } GLvectord;

#ifdef __cplusplus
extern "C" {
#endif

extern void
crMatrixInit(CRmatrix *m);

extern void
crMatrixInitFromString(CRmatrix *m, const char *s);

extern void
crMatrixInitFromFloats(CRmatrix *m, const float *v);

extern void
crMatrixInitFromDoubles(CRmatrix *m, const double *v);

extern void
crMatrixPrint(const char *msg, const CRmatrix *m);

extern void
crMatrixGetFloats(float *values, const CRmatrix *m);

extern int
crMatrixIsEqual(const CRmatrix *m, const CRmatrix *n);

extern int
crMatrixIsIdentity(const CRmatrix *m);

extern int
crMatrixIsOrthographic(const CRmatrix *m);

extern void
crMatrixCopy(CRmatrix *dest, const CRmatrix *src);

extern void
crMatrixMultiply(CRmatrix *p, const CRmatrix *a, const CRmatrix *b);

extern void
crMatrixTransformPointf(const CRmatrix *m, GLvectorf *p);

extern void
crMatrixTransformPointd(const CRmatrix *m, GLvectord *p);

extern void
crMatrixInvertTranspose(CRmatrix *inv, const CRmatrix *mat);

extern void 
crMatrixTranspose(CRmatrix *t, const CRmatrix *m);

extern void
crMatrixTranslate(CRmatrix *m, float x, float y, float z);

extern void
crMatrixRotate(CRmatrix *m, float angle, float x, float y, float z);

extern void
crMatrixScale(CRmatrix *m, float x, float y, float z);

extern void
crMatrixFrustum(CRmatrix *m,
								float left, float right,
								float bottom, float top, 
								float zNear, float zFar);

extern void
crMatrixOrtho(CRmatrix *m,
							float left, float right,
							float bottom, float top,
							float znear, float zfar);

#ifdef __cplusplus
}
#endif

#endif
