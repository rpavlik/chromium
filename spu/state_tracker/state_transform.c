#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "cr_mem.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

#include <math.h>
#include <assert.h>
#ifdef WINDOWS
#pragma warning( disable : 4514 )  
#endif

static const GLmatrix identity_matrix = 
{
	(GLdefault) 1.0, (GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 1.0, (GLdefault) 0.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 1.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 1.0
};

#ifndef M_PI
#define M_PI             3.14159265358979323846
#endif

#define MAX_MATRIX_STACK_DEPTH 512
#define MAX_CLIPPLANES 8

#ifdef GL_DEFAULTTYPE_FLOAT

#define LOADMATRIX(a) \
{ \
GLfloat f[16]; \
f[0] = (a)->m00; f[1] = (a)->m01; f[2] = (a)->m02; f[3] = (a)->m03; \
f[4] = (a)->m10; f[5] = (a)->m11; f[6] = (a)->m12; f[7] = (a)->m13; \
f[8] = (a)->m20; f[9] = (a)->m21; f[10] = (a)->m22; f[11] = (a)->m23; \
f[12] = (a)->m30; f[13] = (a)->m31; f[14] = (a)->m32; f[15] = (a)->m33; \
diff_api.LoadMatrixf((const GLfloat *) f); \
}

#endif

#ifdef GL_DEFAULTTYPE_DOUBLE

#define LOADMATRIX(a) \
{ \
GLdouble f[16]; \
f[0] = (a)->m00; f[1] = (a)->m01; f[2] = (a)->m02; f[3] = (a)->m03; \
f[4] = (a)->m10; f[5] = (a)->m11; f[6] = (a)->m12; f[7] = (a)->m13; \
f[8] = (a)->m20; f[9] = (a)->m21; f[10] = (a)->m22; f[11] = (a)->m23; \
f[12] = (a)->m30; f[13] = (a)->m31; f[14] = (a)->m32; f[15] = (a)->m33; \
diff_api.LoadMatrixd((const GLdouble *) f); \
}
#endif


void crStateTransformInitBits (CRTransformBits *t) 
{
	t->dirty = GLBITS_ONES;
	t->matrix[1] = GLBITS_ONES;
	t->compress = GLBITS_ONES;
}

void crStateTransformInit(CRTransformState *t) 
{
	int i;

	t->mode = GL_MODELVIEW;
	t->matrixid = 0;

	t->maxModelViewStackDepth = MAX_MATRIX_STACK_DEPTH;
	t->maxProjectionStackDepth = MAX_MATRIX_STACK_DEPTH;
	t->maxTextureStackDepth = MAX_MATRIX_STACK_DEPTH;
	t->maxColorStackDepth = MAX_MATRIX_STACK_DEPTH;
	t->maxDepth = t->maxModelViewStackDepth;

	if (t->maxModelViewStackDepth)
		t->modelView = (GLmatrix *) crAlloc (sizeof (GLmatrix) * t->maxModelViewStackDepth);
	if (t->maxProjectionStackDepth)
		t->projection = (GLmatrix *) crAlloc (sizeof (GLmatrix) * t->maxProjectionStackDepth);
	if (t->maxTextureStackDepth)
		t->texture = (GLmatrix *) crAlloc (sizeof (GLmatrix) * t->maxTextureStackDepth);
	if (t->maxColorStackDepth)
		t->color = (GLmatrix *) crAlloc (sizeof (GLmatrix) * t->maxColorStackDepth);

	t->modelView[0] = identity_matrix;
	t->projection[0] = identity_matrix;
	t->texture[0] = identity_matrix;
	t->color[0] = identity_matrix;
	t->m = t->modelView;

	t->modelViewDepth = 0;
	t->projectionDepth = 0;
	t->textureDepth = 0;
	t->colorDepth = 0;
	t->depth = &t->modelViewDepth;

	t->maxClipPlanes = MAX_CLIPPLANES;

	t->clipPlane = NULL;
	t->clip = NULL;
	if (t->maxClipPlanes) 
	{
		t->clipPlane = (GLvectord *) crAlloc (sizeof (GLvectord) * t->maxClipPlanes);
		t->clip = (GLboolean *) crAlloc (sizeof (GLboolean) * t->maxClipPlanes);
		for (i=0; i<t->maxClipPlanes; i++) 
		{
			t->clipPlane[i].x = 0.0f;
			t->clipPlane[i].y = 0.0f;
			t->clipPlane[i].z = 0.0f;
			t->clipPlane[i].w = 0.0f;
			t->clip[i] = GL_FALSE;
		}
	}

	t->transformValid = 0;
}

void crStateTransformUpdateTransform (CRTransformState *t) 
{
	GLmatrix *m1 = &t->projection[t->projectionDepth];
	GLmatrix *m2 = &t->modelView[t->modelViewDepth];
	GLmatrix *m = &(t->transform);
	// I really hate this.
	const GLdefault lm00 = m1->m00; const GLdefault lm01 = m1->m01;  
	const GLdefault lm02 = m1->m02;	const GLdefault lm03 = m1->m03;	
	const GLdefault lm10 = m1->m10;	const GLdefault lm11 = m1->m11;
	const GLdefault lm12 = m1->m12;	const GLdefault lm13 = m1->m13;	
	const GLdefault lm20 = m1->m20;	const GLdefault lm21 = m1->m21;	
	const GLdefault lm22 = m1->m22;	const GLdefault lm23 = m1->m23;	
	const GLdefault lm30 = m1->m30;	const GLdefault lm31 = m1->m31;	
	const GLdefault lm32 = m1->m32;	const GLdefault lm33 = m1->m33;		
	const GLdefault rm00 = m2->m00; const GLdefault rm01 = m2->m01;  
	const GLdefault rm02 = m2->m02;	const GLdefault rm03 = m2->m03;	
	const GLdefault rm10 = m2->m10;	const GLdefault rm11 = m2->m11;	
	const GLdefault rm12 = m2->m12;	const GLdefault rm13 = m2->m13;	
	const GLdefault rm20 = m2->m20;	const GLdefault rm21 = m2->m21;	
	const GLdefault rm22 = m2->m22;	const GLdefault rm23 = m2->m23;	
	const GLdefault rm30 = m2->m30;	const GLdefault rm31 = m2->m31;	
	const GLdefault rm32 = m2->m32;	const GLdefault rm33 = m2->m33;	
	m->m00 = lm00*rm00 + lm10*rm01 + lm20*rm02 + lm30*rm03;	
	m->m01 = lm01*rm00 + lm11*rm01 + lm21*rm02 + lm31*rm03;	
	m->m02 = lm02*rm00 + lm12*rm01 + lm22*rm02 + lm32*rm03;	
	m->m03 = lm03*rm00 + lm13*rm01 + lm23*rm02 + lm33*rm03;	
	m->m10 = lm00*rm10 + lm10*rm11 + lm20*rm12 + lm30*rm13;	
	m->m11 = lm01*rm10 + lm11*rm11 + lm21*rm12 + lm31*rm13;	
	m->m12 = lm02*rm10 + lm12*rm11 + lm22*rm12 + lm32*rm13;	
	m->m13 = lm03*rm10 + lm13*rm11 + lm23*rm12 + lm33*rm13;	
	m->m20 = lm00*rm20 + lm10*rm21 + lm20*rm22 + lm30*rm23;	
	m->m21 = lm01*rm20 + lm11*rm21 + lm21*rm22 + lm31*rm23;	
	m->m22 = lm02*rm20 + lm12*rm21 + lm22*rm22 + lm32*rm23;	
	m->m23 = lm03*rm20 + lm13*rm21 + lm23*rm22 + lm33*rm23;	
	m->m30 = lm00*rm30 + lm10*rm31 + lm20*rm32 + lm30*rm33;	
	m->m31 = lm01*rm30 + lm11*rm31 + lm21*rm32 + lm31*rm33;	
	m->m32 = lm02*rm30 + lm12*rm31 + lm22*rm32 + lm32*rm33;	
	m->m33 = lm03*rm30 + lm13*rm31 + lm23*rm32 + lm33*rm33;
	t->transformValid = 1;
}

void crStateTransformXformPoint(CRTransformState *t, GLvectorf *p) 
{
	GLmatrix *m;
	GLfloat x = p->x;
	GLfloat y = p->y;
	GLfloat z = p->z;
	GLfloat w = p->w;

	if (!t->transformValid)
		crStateTransformUpdateTransform(t);

	m = &(t->transform);

	p->x = m->m00*x + m->m10*y + m->m20*z + m->m30*w;
	p->y = m->m01*x + m->m11*y + m->m21*z + m->m31*w;
	p->z = m->m02*x + m->m12*y + m->m22*z + m->m32*w;
	p->w = m->m03*x + m->m13*y + m->m23*z + m->m33*w;
}

void crStateTransformXformPointMatrixf(GLmatrix *m, GLvectorf *p) 
{
	GLfloat x = p->x;
	GLfloat y = p->y;
	GLfloat z = p->z;
	GLfloat w = p->w;

	p->x = m->m00*x + m->m10*y + m->m20*z + m->m30*w;
	p->y = m->m01*x + m->m11*y + m->m21*z + m->m31*w;
	p->z = m->m02*x + m->m12*y + m->m22*z + m->m32*w;
	p->w = m->m03*x + m->m13*y + m->m23*z + m->m33*w;
}

void crStateTransformXformPointMatrixd(GLmatrix *m, GLvectord *p) {
	GLdouble x = p->x;
	GLdouble y = p->y;
	GLdouble z = p->z;
	GLdouble w = p->w;

	p->x = (GLdouble) (m->m00*x + m->m10*y + m->m20*z + m->m30*w);
	p->y = (GLdouble) (m->m01*x + m->m11*y + m->m21*z + m->m31*w);
	p->z = (GLdouble) (m->m02*x + m->m12*y + m->m22*z + m->m32*w);
	p->w = (GLdouble) (m->m03*x + m->m13*y + m->m23*z + m->m33*w);
}

void crStateTransformInvertTransposeMatrix(GLmatrix *inv, GLmatrix *mat) {
	/* Taken from Pomegranate code, trans.c.
	 * Note: We have our data structures reversed
	 */
	GLdefault m00 = mat->m00;
	GLdefault m01 = mat->m10;
	GLdefault m02 = mat->m20;
	GLdefault m03 = mat->m30;

	GLdefault m10 = mat->m01;
	GLdefault m11 = mat->m11;
	GLdefault m12 = mat->m21;
	GLdefault m13 = mat->m31;

	GLdefault m20 = mat->m02;
	GLdefault m21 = mat->m12;
	GLdefault m22 = mat->m22;
	GLdefault m23 = mat->m32;

	GLdefault m30 = mat->m03;
	GLdefault m31 = mat->m13;
	GLdefault m32 = mat->m23;
	GLdefault m33 = mat->m33;

#define det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3) \
	(a1 * (b2 * c3 - b3 * c2) + \
	 b1 * (c2 * a3 - a2 * c3) + \
	 c1 * (a2 * b3 - a3 * b2))

  GLdefault cof00 =  det3x3( m11, m12, m13,
                             m21, m22, m23,
                             m31, m32, m33 );

  GLdefault cof01 = -det3x3( m12, m13, m10,
                             m22, m23, m20,
                             m32, m33, m30 );

  GLdefault cof02 = det3x3( m13, m10, m11,
                             m23, m20, m21,
                             m33, m30, m31 );

  GLdefault cof03 = -det3x3( m10, m11, m12,
                             m20, m21, m22,
                             m30, m31, m32 );


  GLdefault inv_det = ((GLdefault) 1.0) / ( m00 * cof00 + m01 * cof01 +
                                            m02 * cof02 + m03 * cof03 );


  GLdefault cof10 = -det3x3( m21, m22, m23,
                             m31, m32, m33,
                             m01, m02, m03 );

  GLdefault cof11 = det3x3( m22, m23, m20,
                             m32, m33, m30,
                             m02, m03, m00 );

  GLdefault cof12 = -det3x3( m23, m20, m21,
                             m33, m30, m31,
                             m03, m00, m01 );

  GLdefault cof13 = det3x3( m20, m21, m22,
                             m30, m31, m32,
                             m00, m01, m02 );



  GLdefault cof20 = det3x3( m31, m32, m33,
                             m01, m02, m03,
                             m11, m12, m13 );

  GLdefault cof21 = -det3x3( m32, m33, m30,
                             m02, m03, m00,
                             m12, m13, m10 );

  GLdefault cof22 = det3x3( m33, m30, m31,
                             m03, m00, m01,
                             m13, m10, m11 );

  GLdefault cof23 = -det3x3( m30, m31, m32,
                             m00, m01, m02,
                             m10, m11, m12 );


  GLdefault cof30 = -det3x3( m01, m02, m03,
                             m11, m12, m13,
                             m21, m22, m23 );

  GLdefault cof31 = det3x3( m02, m03, m00,
                             m12, m13, m10,
                             m22, m23, m20 );

  GLdefault cof32 = -det3x3( m03, m00, m01,
                             m13, m10, m11,
                             m23, m20, m21 );

  GLdefault cof33 = det3x3( m00, m01, m02,
                             m10, m11, m12,
                             m20, m21, m22 );

#undef det3x3

	/* Perform transpose in asignment */

	inv->m00 = cof00 * inv_det; inv->m10 = cof01 * inv_det;
	inv->m20 = cof02 * inv_det; inv->m30 = cof03 * inv_det;

	inv->m01 = cof10 * inv_det; inv->m11 = cof11 * inv_det;
	inv->m21 = cof12 * inv_det; inv->m31 = cof13 * inv_det;

	inv->m02 = cof20 * inv_det; inv->m12 = cof21 * inv_det;
	inv->m22 = cof22 * inv_det; inv->m32 = cof23 * inv_det;

	inv->m03 = cof30 * inv_det; inv->m13 = cof31 * inv_det;
	inv->m23 = cof32 * inv_det; inv->m33 = cof33 * inv_det;
}


void STATE_APIENTRY crStateClipPlane (GLenum plane, const GLdouble *equation) {
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &g->transform;
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLvectord e;
	int i;
	GLmatrix inv;

	e.x = equation[0];
	e.y = equation[1];
	e.z = equation[2];
	e.w = equation[3];

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "ClipPlane called in begin/end");
		return;
	}

	FLUSH();

	i = plane - GL_CLIP_PLANE0;
	if (i >= t->maxClipPlanes)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "ClipPlane called with bad enumerant: %d", plane);
		return;
	}

	crStateTransformInvertTransposeMatrix (&inv, &(t->modelView[t->modelViewDepth]));
	crStateTransformXformPointMatrixd (&inv, &e);
	t->clipPlane[i] = e;
	tb->clipPlane = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateMatrixMode(GLenum e) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "MatrixMode called in begin/end");
		return;
	}

	FLUSH();

	switch (e) 
	{
		case GL_MODELVIEW:
			t->m = &(t->modelView[t->modelViewDepth]);
			t->depth = &t->modelViewDepth;
			t->maxDepth = t->maxModelViewStackDepth;
			t->mode = GL_MODELVIEW;
			t->matrixid = 0;
			break;
		case GL_PROJECTION:
			t->m = &(t->projection[t->projectionDepth]);
			t->depth = &t->projectionDepth;
			t->maxDepth = t->maxProjectionStackDepth;
			t->mode = GL_PROJECTION;
			t->matrixid = 1;
			break;
		case GL_TEXTURE:
			t->m = &(t->texture[t->textureDepth]);
			t->depth = &t->textureDepth;
			t->maxDepth = t->maxTextureStackDepth;
			t->mode = GL_TEXTURE;
			t->matrixid = 2;
			break;
		case GL_COLOR:
			t->m = &(t->color[t->colorDepth]);
			t->depth = &t->colorDepth;
			t->maxDepth = t->maxColorStackDepth;
			t->mode = GL_COLOR;
			t->matrixid = 3;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid matrix mode: %d", e);
			return;
	}
	tb->mode = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateLoadIdentity() 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "LoadIdentity called in begin/end");
		return;
	}

	FLUSH();

	*(t->m) = identity_matrix;
	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;

}

void STATE_APIENTRY crStatePopMatrix() 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &g->transform;
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PopMatrix called in begin/end");
		return;
	}

	FLUSH();

	if (*(t->depth) == 0)
	{
		crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "PopMatrix of empty stack.");
		return;
	}

	if ( *(t->depth) < 0 && *(t->depth) >= t->maxDepth ) 
	{
		crError( "Invalid depth in PopMatrix: %d", *(t->depth) );
	}

	t->m--;
	(*(t->depth))--;

	t->transformValid = 0;

	tb->matrix[t->matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStatePushMatrix() 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &g->transform;
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PushMatrix called in begin/end");
		return;
	}

	FLUSH();

	if (*(t->depth)+1 == t->maxDepth)
	{
		crStateError(__LINE__, __FILE__, GL_STACK_OVERFLOW, "PushMatrix pass the end of allocated stack");
		return;
	}

	if ( *(t->depth) < 0 && *(t->depth) >= t->maxDepth ) 
	{
		crError( "Bogus depth in PushMatrix: %d", *(t->depth) );
	}

	/* Perform the copy */
	t->m[1] = t->m[0];


	/* Move the stack pointer */
	t->m++;
	(*t->depth)++;

	tb->matrix[t->matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}


void STATE_APIENTRY crStateLoadMatrixf(const GLfloat *m1) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLmatrix *m;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "LoadMatrixf called in begin/end");
		return;
	}

	FLUSH();

	m = t->m;
	m->m00 = (GLdefault) m1[0];	
	m->m01 = (GLdefault) m1[1];		
	m->m02 = (GLdefault) m1[2];	
	m->m03 = (GLdefault) m1[3];	
	m->m10 = (GLdefault) m1[4];	
	m->m11 = (GLdefault) m1[5];		
	m->m12 = (GLdefault) m1[6];	
	m->m13 = (GLdefault) m1[7];	
	m->m20 = (GLdefault) m1[8];		
	m->m21 = (GLdefault) m1[9];	
	m->m22 = (GLdefault) m1[10];	
	m->m23 = (GLdefault) m1[11];	
	m->m30 = (GLdefault) m1[12];	
	m->m31 = (GLdefault) m1[13];	
	m->m32 = (GLdefault) m1[14];	
	m->m33 = (GLdefault) m1[15];	

	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;

}

void STATE_APIENTRY crStateLoadMatrixd(const GLdouble *m1) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLmatrix *m;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "LoadMatrixd called in begin/end");
		return;
	}

	FLUSH();

	m = t->m;
	m->m00 = (GLdefault) m1[0];	
	m->m01 = (GLdefault) m1[1];		
	m->m02 = (GLdefault) m1[2];	
	m->m03 = (GLdefault) m1[3];	
	m->m10 = (GLdefault) m1[4];	
	m->m11 = (GLdefault) m1[5];		
	m->m12 = (GLdefault) m1[6];	
	m->m13 = (GLdefault) m1[7];	
	m->m20 = (GLdefault) m1[8];		
	m->m21 = (GLdefault) m1[9];	
	m->m22 = (GLdefault) m1[10];	
	m->m23 = (GLdefault) m1[11];	
	m->m30 = (GLdefault) m1[12];	
	m->m31 = (GLdefault) m1[13];	
	m->m32 = (GLdefault) m1[14];	
	m->m33 = (GLdefault) m1[15];	

	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

/* This code is based on the Pomegranate stuff.
 ** The theory is that by preloading everything,
 ** the compiler could do optimizations that 
 ** were not doable in the array version
 ** I'm not too sure with a PII with 4 registers
 ** that this really helps.
 */	
void STATE_APIENTRY crStateMultMatrixf(const GLfloat *m1) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLmatrix *m = t->m;
	const GLdefault lm00 = m->m00;  
	const GLdefault lm01 = m->m01;  
	const GLdefault lm02 = m->m02;	
	const GLdefault lm03 = m->m03;	
	const GLdefault lm10 = m->m10;	
	const GLdefault lm11 = m->m11;	
	const GLdefault lm12 = m->m12;	
	const GLdefault lm13 = m->m13;	
	const GLdefault lm20 = m->m20;	
	const GLdefault lm21 = m->m21;	
	const GLdefault lm22 = m->m22;	
	const GLdefault lm23 = m->m23;	
	const GLdefault lm30 = m->m30;	
	const GLdefault lm31 = m->m31;	
	const GLdefault lm32 = m->m32;		
	const GLdefault lm33 = m->m33;		
	const GLdefault rm00 = (GLdefault) m1[0];	
	const GLdefault rm01 = (GLdefault) m1[1];	
	const GLdefault rm02 = (GLdefault) m1[2];	
	const GLdefault rm03 = (GLdefault) m1[3];	
	const GLdefault rm10 = (GLdefault) m1[4];	
	const GLdefault rm11 = (GLdefault) m1[5];		
	const GLdefault rm12 = (GLdefault) m1[6];	
	const GLdefault rm13 = (GLdefault) m1[7];	
	const GLdefault rm20 = (GLdefault) m1[8];	
	const GLdefault rm21 = (GLdefault) m1[9];	
	const GLdefault rm22 = (GLdefault) m1[10];	
	const GLdefault rm23 = (GLdefault) m1[11];	
	const GLdefault rm30 = (GLdefault) m1[12];	
	const GLdefault rm31 = (GLdefault) m1[13];	
	const GLdefault rm32 = (GLdefault) m1[14];	
	const GLdefault rm33 = (GLdefault) m1[15];	

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "MultMatrixf called in begin/end");
		return;
	}

	FLUSH();

	m->m00 = lm00*rm00 + lm10*rm01 + lm20*rm02 + lm30*rm03;	
	m->m01 = lm01*rm00 + lm11*rm01 + lm21*rm02 + lm31*rm03;	
	m->m02 = lm02*rm00 + lm12*rm01 + lm22*rm02 + lm32*rm03;	
	m->m03 = lm03*rm00 + lm13*rm01 + lm23*rm02 + lm33*rm03;	
	m->m10 = lm00*rm10 + lm10*rm11 + lm20*rm12 + lm30*rm13;	
	m->m11 = lm01*rm10 + lm11*rm11 + lm21*rm12 + lm31*rm13;	
	m->m12 = lm02*rm10 + lm12*rm11 + lm22*rm12 + lm32*rm13;	
	m->m13 = lm03*rm10 + lm13*rm11 + lm23*rm12 + lm33*rm13;	
	m->m20 = lm00*rm20 + lm10*rm21 + lm20*rm22 + lm30*rm23;	
	m->m21 = lm01*rm20 + lm11*rm21 + lm21*rm22 + lm31*rm23;	
	m->m22 = lm02*rm20 + lm12*rm21 + lm22*rm22 + lm32*rm23;	
	m->m23 = lm03*rm20 + lm13*rm21 + lm23*rm22 + lm33*rm23;	
	m->m30 = lm00*rm30 + lm10*rm31 + lm20*rm32 + lm30*rm33;	
	m->m31 = lm01*rm30 + lm11*rm31 + lm21*rm32 + lm31*rm33;	
	m->m32 = lm02*rm30 + lm12*rm31 + lm22*rm32 + lm32*rm33;	
	m->m33 = lm03*rm30 + lm13*rm31 + lm23*rm32 + lm33*rm33;

	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateMultMatrixd(const GLdouble *m1) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLmatrix *m = t->m;
	const GLdefault lm00 = m->m00;  
	const GLdefault lm01 = m->m01;  
	const GLdefault lm02 = m->m02;	
	const GLdefault lm03 = m->m03;	
	const GLdefault lm10 = m->m10;	
	const GLdefault lm11 = m->m11;	
	const GLdefault lm12 = m->m12;	
	const GLdefault lm13 = m->m13;	
	const GLdefault lm20 = m->m20;	
	const GLdefault lm21 = m->m21;	
	const GLdefault lm22 = m->m22;	
	const GLdefault lm23 = m->m23;	
	const GLdefault lm30 = m->m30;	
	const GLdefault lm31 = m->m31;	
	const GLdefault lm32 = m->m32;		
	const GLdefault lm33 = m->m33;		
	const GLdefault rm00 = (GLdefault) m1[0];	
	const GLdefault rm01 = (GLdefault) m1[1];	
	const GLdefault rm02 = (GLdefault) m1[2];	
	const GLdefault rm03 = (GLdefault) m1[3];	
	const GLdefault rm10 = (GLdefault) m1[4];	
	const GLdefault rm11 = (GLdefault) m1[5];		
	const GLdefault rm12 = (GLdefault) m1[6];	
	const GLdefault rm13 = (GLdefault) m1[7];	
	const GLdefault rm20 = (GLdefault) m1[8];	
	const GLdefault rm21 = (GLdefault) m1[9];	
	const GLdefault rm22 = (GLdefault) m1[10];	
	const GLdefault rm23 = (GLdefault) m1[11];	
	const GLdefault rm30 = (GLdefault) m1[12];	
	const GLdefault rm31 = (GLdefault) m1[13];	
	const GLdefault rm32 = (GLdefault) m1[14];	
	const GLdefault rm33 = (GLdefault) m1[15];	

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "MultMatrixd called in begin/end");
		return;
	}

	FLUSH();

	m->m00 = lm00*rm00 + lm10*rm01 + lm20*rm02 + lm30*rm03;	
	m->m01 = lm01*rm00 + lm11*rm01 + lm21*rm02 + lm31*rm03;	
	m->m02 = lm02*rm00 + lm12*rm01 + lm22*rm02 + lm32*rm03;	
	m->m03 = lm03*rm00 + lm13*rm01 + lm23*rm02 + lm33*rm03;	
	m->m10 = lm00*rm10 + lm10*rm11 + lm20*rm12 + lm30*rm13;	
	m->m11 = lm01*rm10 + lm11*rm11 + lm21*rm12 + lm31*rm13;	
	m->m12 = lm02*rm10 + lm12*rm11 + lm22*rm12 + lm32*rm13;	
	m->m13 = lm03*rm10 + lm13*rm11 + lm23*rm12 + lm33*rm13;	
	m->m20 = lm00*rm20 + lm10*rm21 + lm20*rm22 + lm30*rm23;	
	m->m21 = lm01*rm20 + lm11*rm21 + lm21*rm22 + lm31*rm23;	
	m->m22 = lm02*rm20 + lm12*rm21 + lm22*rm22 + lm32*rm23;	
	m->m23 = lm03*rm20 + lm13*rm21 + lm23*rm22 + lm33*rm23;	
	m->m30 = lm00*rm30 + lm10*rm31 + lm20*rm32 + lm30*rm33;	
	m->m31 = lm01*rm30 + lm11*rm31 + lm21*rm32 + lm31*rm33;	
	m->m32 = lm02*rm30 + lm12*rm31 + lm22*rm32 + lm32*rm33;	
	m->m33 = lm03*rm30 + lm13*rm31 + lm23*rm32 + lm33*rm33;

	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateTranslatef(GLfloat x_arg, GLfloat y_arg, GLfloat z_arg) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLmatrix *m = t->m;
	const GLdefault x = (GLdefault) x_arg;
	const GLdefault y = (GLdefault) y_arg;
	const GLdefault z = (GLdefault) z_arg;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Translatef called in begin/end");
		return;
	}

	FLUSH();

	m->m30 = m->m00*x + m->m10*y + m->m20*z + m->m30;
	m->m31 = m->m01*x + m->m11*y + m->m21*z + m->m31;
	m->m32 = m->m02*x + m->m12*y + m->m22*z + m->m32;
	m->m33 = m->m03*x + m->m13*y + m->m23*z + m->m33;

	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}


void STATE_APIENTRY crStateTranslated(GLdouble x_arg, GLdouble y_arg, GLdouble z_arg) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLmatrix *m = t->m;
	const GLdefault x = (GLdefault) x_arg;
	const GLdefault y = (GLdefault) y_arg;
	const GLdefault z = (GLdefault) z_arg;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Translated called in begin/end");
		return;
	}

	FLUSH();

	m->m30 = m->m00*x + m->m10*y + m->m20*z + m->m30;
	m->m31 = m->m01*x + m->m11*y + m->m21*z + m->m31;
	m->m32 = m->m02*x + m->m12*y + m->m22*z + m->m32;
	m->m33 = m->m03*x + m->m13*y + m->m23*z + m->m33;

	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}	

/* TODO: use lookup tables for cosine and sine functions */
/* TODO: we need a fast sqrt function SIMD?*/
/* TODO: don't call multmatrix. Do your own multiply. */

void STATE_APIENTRY crStateRotatef(GLfloat ang, GLfloat x, GLfloat y, GLfloat z) 
{
	const GLfloat c = (GLfloat) cos(ang*M_PI/180.0f);			
	const GLfloat one_minus_c = 1.0f - c;	
	const GLfloat s = (GLfloat) sin(ang*M_PI/180.0f);			
	const GLfloat v_len = (GLfloat) sqrt (x*x + y*y + z*z);	
	GLfloat rot[16];				
	GLfloat x_one_minus_c;	
	GLfloat y_one_minus_c;	
	GLfloat z_one_minus_c;	

	// Begin/end Checking and flushing will be done by MultMatrix.

	if (v_len == 0.0f)
		return;

	/* Normalize the vector */	
	if (v_len != 1.0f) {	

		x /= v_len;				
		y /= v_len;					
		z /= v_len;				
	}							
	/* compute some common values */	
	x_one_minus_c = x * one_minus_c;	
	y_one_minus_c = y * one_minus_c;	
	z_one_minus_c = z * one_minus_c;	
	/* Generate the terms of the rotation matrix	
	 ** from pg 325 OGL 1.1 Blue Book.				
	 */												
	rot[0] = x*x_one_minus_c + c;					
	rot[1] = x*y_one_minus_c + z*s;				
	rot[2] = x*z_one_minus_c - y*s;				
	rot[3] = 0.0f;						
	rot[4] = y*x_one_minus_c - z*s;				
	rot[5] = y*y_one_minus_c + c;					
	rot[6] = y*z_one_minus_c + x*s;				
	rot[7] = 0.0f;						
	rot[8] = z*x_one_minus_c + y*s;				
	rot[9] = z*y_one_minus_c - x*s;				
	rot[10] = z*z_one_minus_c + c;						
	rot[11] = 0.0f;						
	rot[12] = 0.0f;						
	rot[13] = 0.0f;						
	rot[14] = 0.0f;						
	rot[15] = 1.0f;						
	crStateMultMatrixf((const GLfloat *) rot);
}

void STATE_APIENTRY crStateRotated(GLdouble ang, GLdouble x, GLdouble y, GLdouble z) 
{
	const GLdouble c = (GLdouble) cos(ang*M_PI/180.0);			
	const GLdouble one_minus_c = 1.0 - c;	
	const GLdouble s = (GLdouble) sin(ang*M_PI/180.0f);			
	const GLdouble v_len = (GLdouble) sqrt (x*x + y*y + z*z);	
	GLdouble rot[16];				
	GLdouble x_one_minus_c;	
	GLdouble y_one_minus_c;	
	GLdouble z_one_minus_c;	

	// Begin/end Checking and flushing will be done by MultMatrix.


	/* Normalize the vector */	
	if (v_len != 1.0f) {	

		x /= v_len;				
		y /= v_len;					
		z /= v_len;				
	}							
	/* compute some common values */	
	x_one_minus_c = x * one_minus_c;	
	y_one_minus_c = y * one_minus_c;	
	z_one_minus_c = z * one_minus_c;	
	/* Generate the terms of the rotation matrix	
	 ** from pg 325 OGL 1.1 Blue Book.				
	 */												
	rot[0] = x*x_one_minus_c + c;					
	rot[1] = x*y_one_minus_c + z*s;				
	rot[2] = x*z_one_minus_c - y*s;				
	rot[3] = 0.0;						
	rot[4] = y*x_one_minus_c - z*s;				
	rot[5] = y*y_one_minus_c + c;					
	rot[6] = y*z_one_minus_c + x*s;				
	rot[7] = 0.0;						
	rot[8] = z*x_one_minus_c + y*s;				
	rot[9] = z*y_one_minus_c - x*s;				
	rot[10] = z*z_one_minus_c + c;						
	rot[11] = 0.0;						
	rot[12] = 0.0;						
	rot[13] = 0.0;						
	rot[14] = 0.0;						
	rot[15] = 1.0;						
	crStateMultMatrixd((const GLdouble *) rot);
}	

void STATE_APIENTRY crStateScalef (GLfloat x_arg, GLfloat y_arg, GLfloat z_arg) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLmatrix *m = t->m;
	const GLdefault x = (GLdefault) x_arg;	
	const GLdefault y = (GLdefault) y_arg;	
	const GLdefault z = (GLdefault) z_arg;	

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Scalef called in begin/end");
		return;
	}

	FLUSH();

	m->m00 *= x;	
	m->m01 *= x;	
	m->m02 *= x;		
	m->m03 *= x;	
	m->m10 *= y;	
	m->m11 *= y;	
	m->m12 *= y;	
	m->m13 *= y;	
	m->m20 *= z;	
	m->m21 *= z;	
	m->m22 *= z;	
	m->m23 *= z;	

	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateScaled (GLdouble x_arg, GLdouble y_arg, GLdouble z_arg) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLmatrix *m = t->m;
	const GLdefault x = (GLdefault) x_arg;	
	const GLdefault y = (GLdefault) y_arg;	
	const GLdefault z = (GLdefault) z_arg;	

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Scaled called in begin/end");
		return;
	}

	FLUSH();

	m->m00 *= x;	
	m->m01 *= x;	
	m->m02 *= x;		
	m->m03 *= x;	
	m->m10 *= y;	
	m->m11 *= y;	
	m->m12 *= y;	
	m->m13 *= y;	
	m->m20 *= z;	
	m->m21 *= z;	
	m->m22 *= z;	
	m->m23 *= z;

	t->transformValid = 0;

	tb->matrix[g->transform.matrixid] = g->neg_bitid;
	tb->compress = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateFrustum (	GLdouble left, GLdouble right,
		GLdouble bottom, GLdouble top, 
		GLdouble zNear, GLdouble zFar) 
{
	GLdouble m[16];

	// Begin/end Checking and flushing will be done by MultMatrix.

	/* Build the frustum matrix
	 ** from pg 163 OGL 1.1 Blue Book
	 */
	m[0] = (2.0*zNear)/(right-left);
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;

	m[4] = 0.0;
	m[5] = (2.0*zNear)/(top-bottom);
	m[6] = 0.0;
	m[7] = 0.0;

	m[8] = (right+left)/(right-left);
	m[9] = (top+bottom)/(top-bottom);
	m[10] = (-zNear-zFar)/(zFar-zNear);
	m[11] = -1.0;

	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = (2.0*zFar*zNear)/(zNear-zFar);
	m[15] = 0.0;

	crStateMultMatrixd((const GLdouble *) m);
}

void STATE_APIENTRY crStateOrtho ( GLdouble left, GLdouble right,
		GLdouble bottom, GLdouble top,
		GLdouble znear, GLdouble zfar) 
{

	GLdouble m[16];	

	// Begin/end Checking and flushing will be done by MultMatrix.

	m[0] = 2.0 / (right - left);
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;

	m[4] = 0.0;
	m[5] = 2.0 / (top - bottom);
	m[6] = 0.0;
	m[7] = 0.0;

	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = -2.0 / (zfar - znear);
	m[11] = 0.0;

	m[12] = -(right + left) / (right - left);
	m[13] = -(top + bottom) / (top - bottom);
	m[14] = -(zfar + znear) / (zfar - znear);
	m[15] = 1.0;

	crStateMultMatrixd((const GLdouble *) m);
}

void  STATE_APIENTRY crStateGetClipPlane (GLenum plane, GLdouble *equation) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &g->transform;
	int i;
	
	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"glGetClipPlane called in begin/end");
		return;
	}

	i = plane - GL_CLIP_PLANE0;
	if (i >= t->maxClipPlanes)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
			"GetClipPlane called with bad enumerant: %d", plane);
		return;
	}

	equation[0] = t->clipPlane[i].x;
	equation[1] = t->clipPlane[i].x;
	equation[2] = t->clipPlane[i].x;
	equation[3] = t->clipPlane[i].x;
}

void crStateTransformSwitch (CRTransformBits *t, GLbitvalue bitID, 
						 CRTransformState *from, CRTransformState *to) 
{
	GLbitvalue nbitID = ~bitID;
	int i;

	if (t->clipPlane & bitID) {
		for (i=0; i<from->maxClipPlanes; i++) {
			if (from->clipPlane[i].x != to->clipPlane[i].x ||
				from->clipPlane[i].y != to->clipPlane[i].y ||
				from->clipPlane[i].z != to->clipPlane[i].z ||
				from->clipPlane[i].w != to->clipPlane[i].w) {

				GLdouble cp[4];
				cp[0] = to->clipPlane[i].x;
				cp[1] = to->clipPlane[i].y;
				cp[2] = to->clipPlane[i].z;
				cp[3] = to->clipPlane[i].w;

				diff_api.MatrixMode (GL_MODELVIEW);
				diff_api.PushMatrix();
				diff_api.LoadIdentity();
				diff_api.ClipPlane(GL_CLIP_PLANE0 + i, (const GLdouble *)(cp));
				diff_api.PopMatrix();

				t->clipPlane = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
		}
		t->clipPlane &= nbitID;
	}

	if (t->matrix[0] & bitID) {
		if (memcmp (from->modelView+from->modelViewDepth,
										to->modelView+to->modelViewDepth,
										sizeof (GLmatrix))) {

			diff_api.MatrixMode(GL_MODELVIEW);		
			LOADMATRIX(to->modelView+to->modelViewDepth);

			t->matrix[0] = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->matrix[0] &= nbitID;
	}

	if (t->matrix[1] & bitID) {
		if (memcmp (from->projection+from->projectionDepth,
					to->projection+to->projectionDepth,
					sizeof (GLmatrix))) {

			diff_api.MatrixMode(GL_PROJECTION);		
			LOADMATRIX(to->projection+to->projectionDepth);
		
			t->matrix[1] = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->matrix[1] &= nbitID;
	}

	if (t->matrix[2] & bitID) {
		if (memcmp (from->texture+from->textureDepth,
					to->texture+to->textureDepth,
					sizeof (GLmatrix))) {

			diff_api.MatrixMode(GL_TEXTURE_MATRIX);		
			LOADMATRIX(to->texture+to->textureDepth);
		
			t->matrix[2] = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->matrix[2] &= nbitID;
	}

	if (t->matrix[3] & bitID) {
		if (memcmp (from->color+from->colorDepth,
					to->color+to->colorDepth,
					sizeof (GLmatrix))) {

			diff_api.MatrixMode(GL_COLOR);		
			LOADMATRIX(to->color+to->colorDepth);
		
			t->matrix[3] = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->matrix[3] &= nbitID;
	}

/*  HACK: Don't treat MatrixMode as stand alone state.
	if (t->mode & bitID) {
		if (force_mode || from->mode != to->mode) {
			diff_api.MatrixMode(to->mode);
			t->mode = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->mode &= nbitID;
	}
*/

	to->transformValid = 0;
	t->dirty &= nbitID;
}

void crStateTransformDiff(CRTransformBits *t, GLbitvalue bitID, 
						 CRTransformState *from, CRTransformState *to) 
{
	GLbitvalue nbitID = ~bitID;
	GLint i;

	if (t->clipPlane & bitID) {
		for (i=0; i<from->maxClipPlanes; i++) {
			if (from->clipPlane[i].x != to->clipPlane[i].x ||
				from->clipPlane[i].y != to->clipPlane[i].y ||
				from->clipPlane[i].z != to->clipPlane[i].z ||
				from->clipPlane[i].w != to->clipPlane[i].w) {
				
				GLdouble cp[4];
				cp[0] = to->clipPlane[i].x;
				cp[1] = to->clipPlane[i].y;
				cp[2] = to->clipPlane[i].z;
				cp[3] = to->clipPlane[i].w;

				diff_api.MatrixMode (GL_MODELVIEW);
				diff_api.PushMatrix();
				diff_api.LoadIdentity();
				diff_api.ClipPlane(GL_CLIP_PLANE0 + i, (const GLdouble *)(cp));
				diff_api.PopMatrix();
				from->clipPlane[i] = to->clipPlane[i];
			}
		}
		t->clipPlane &= nbitID;
	}
	
	if (t->matrix[0] & bitID) {
		if (memcmp (from->modelView+from->modelViewDepth,
										to->modelView+to->modelViewDepth,
										sizeof (GLmatrix))) {

			diff_api.MatrixMode(GL_MODELVIEW);		
			LOADMATRIX(to->modelView+to->modelViewDepth);

			memcpy((void *) from->modelView, (const void *) to->modelView,
					sizeof (from->modelView[0]) * (to->modelViewDepth + 1));
			from->modelViewDepth = to->modelViewDepth;
		}
		t->matrix[0] &= nbitID;
	}

	if (t->matrix[1] & bitID) {
		diff_api.MatrixMode(GL_PROJECTION);		
		LOADMATRIX(to->projection+to->projectionDepth);

		memcpy((void *) from->projection, (const void *) to->projection,
			sizeof (from->projection[0]) * (to->projectionDepth + 1));
		from->projectionDepth = to->projectionDepth;
		t->matrix[1] &= nbitID;
	}

	if (t->matrix[2] & bitID) {
		if (memcmp (from->texture+from->textureDepth,
					to->texture+to->textureDepth,
					sizeof (GLmatrix))) {

			diff_api.MatrixMode(GL_TEXTURE_MATRIX);		
			LOADMATRIX(to->texture+to->textureDepth);

			memcpy((void *) from->texture, (const void *) to->texture,
					sizeof (from->texture[0]) * (to->textureDepth + 1));
			from->textureDepth = to->textureDepth;
		}
		t->matrix[2] &= nbitID;
	}

	if (t->matrix[3] & bitID) {
		if (memcmp (from->color+from->colorDepth,
					to->color+to->colorDepth,
					sizeof (GLmatrix))) {
			diff_api.MatrixMode(GL_COLOR);		
			LOADMATRIX(to->color+to->colorDepth);

			memcpy((void *) from->color, (const void *) to->color,
					sizeof (from->color[0]) * (to->colorDepth + 1));
			from->colorDepth = to->colorDepth;
		}
		t->matrix[3] &= nbitID;
	}

	/* HACK: Don't treat MatrixMode as stand alone state.
	if (t->mode & bitID) {
		if (force_mode || from->mode != to->mode) {
			diff_api.MatrixMode(to->mode);
			from->mode = to->mode;
		}
		t->mode &= nbitID;
	}
	*/
	
	if (t->enable & bitID) {
		for (i=0; i<from->maxClipPlanes; i++) {
			if (from->clip[i] != to->clip[i]) {
				if (to->clip[i] == GL_TRUE)
					diff_api.Enable(GL_CLIP_PLANE0 + i);
				else
					diff_api.Disable(GL_CLIP_PLANE0 + i);
				from->clip[i] = to->clip[i];
			}
		}
		t->enable &= nbitID;
	}

	to->transformValid = 0;
	t->dirty &= nbitID;
}
