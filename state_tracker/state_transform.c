/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_mem.h"
#include "state.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

#include <math.h>
#include <assert.h>
#ifdef WINDOWS
#pragma warning( disable : 4514 )  
#endif

static const CRmatrix identity_matrix = 
{
	(GLdefault) 1.0, (GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 1.0, (GLdefault) 0.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 1.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 1.0
};

#ifndef M_PI
#define M_PI             3.14159265358979323846
#endif

#ifdef CR_DEFAULTTYPE_FLOAT

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

#ifdef CR_DEFAULTTYPE_DOUBLE

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


#if 0
/* useful for debugging */
static void PrintMatrix( const char *msg, const CRmatrix *m )
{
   printf("%s\n", msg);
   printf("  %f %f %f %f\n", m->m00, m->m01, m->m02, m->m03);
   printf("  %f %f %f %f\n", m->m10, m->m11, m->m12, m->m13);
   printf("  %f %f %f %f\n", m->m20, m->m21, m->m22, m->m23);
   printf("  %f %f %f %f\n", m->m30, m->m31, m->m32, m->m33);
}
#endif

static void _math_transposef( GLfloat to[16], const GLfloat from[16] )
{
   to[0] = from[0];
   to[1] = from[4];
   to[2] = from[8];
   to[3] = from[12];
   to[4] = from[1];
   to[5] = from[5];
   to[6] = from[9];
   to[7] = from[13];
   to[8] = from[2];
   to[9] = from[6];
   to[10] = from[10];
   to[11] = from[14];
   to[12] = from[3];
   to[13] = from[7];
   to[14] = from[11];
   to[15] = from[15];
}

static void _math_transposed( GLdouble to[16], const GLdouble from[16] )
{
	to[0] = from[0];
	to[1] = from[4];
	to[2] = from[8];
	to[3] = from[12];
	to[4] = from[1];
	to[5] = from[5];
	to[6] = from[9];
	to[7] = from[13];
	to[8] = from[2];
	to[9] = from[6];
	to[10] = from[10];
	to[11] = from[14];
	to[12] = from[3];
	to[13] = from[7];
	to[14] = from[11];
	to[15] = from[15];
}


static void
init_matrix_stack(CRMatrixStack *stack, int maxDepth)
{
	stack->maxDepth = maxDepth;
	stack->depth = 0;
	stack->stack = crAlloc(maxDepth * sizeof(CRmatrix));
	stack->stack[0] = identity_matrix;
	stack->top = stack->stack;
}

static void
free_matrix_stack_data(CRMatrixStack *stack)
{
	crFree(stack->stack);
}

void crStateTransformDestroy(CRContext *ctx)
{
	CRTransformState *t = &ctx->transform;
	unsigned int i;

	free_matrix_stack_data(&(t->modelViewStack));
	free_matrix_stack_data(&(t->projectionStack));
	free_matrix_stack_data(&(t->colorStack));
	for (i = 0 ; i < ctx->limits.maxTextureUnits; i++)
		free_matrix_stack_data(&(t->textureStack[i]));
	for (i = 0; i < CR_MAX_PROGRAM_MATRICES; i++)
		free_matrix_stack_data(&(t->programStack[i]));

	crFree( t->clipPlane );
	crFree( t->clip );
}

void crStateTransformInit(CRContext *ctx)
{
	CRLimitsState *limits = &ctx->limits;
	CRTransformState *t = &ctx->transform;
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	unsigned int i;

	t->matrixMode = GL_MODELVIEW;
	RESET(tb->matrixMode, ctx->bitid);

	init_matrix_stack(&t->modelViewStack, CR_MAX_MODELVIEW_STACK_DEPTH);
	init_matrix_stack(&t->projectionStack, CR_MAX_PROJECTION_STACK_DEPTH);
	init_matrix_stack(&t->colorStack, CR_MAX_COLOR_STACK_DEPTH);
	for (i = 0 ; i < limits->maxTextureUnits ; i++)
		init_matrix_stack(&t->textureStack[i], CR_MAX_TEXTURE_STACK_DEPTH);
	for (i = 0 ; i < CR_MAX_PROGRAM_MATRICES ; i++)
		init_matrix_stack(&t->programStack[i], CR_MAX_PROGRAM_MATRIX_STACK_DEPTH);
	t->currentStack = &(t->modelViewStack);

	/* dirty bits */
	RESET(tb->modelviewMatrix, ctx->bitid);
	RESET(tb->projectionMatrix, ctx->bitid);
	RESET(tb->colorMatrix, ctx->bitid);
	RESET(tb->textureMatrix, ctx->bitid);
	RESET(tb->programMatrix, ctx->bitid);
	tb->currentMatrix = tb->modelviewMatrix;

	t->normalize = GL_FALSE;
	RESET(tb->enable, ctx->bitid);

	t->clipPlane = (GLvectord *) crCalloc (sizeof (GLvectord) * CR_MAX_CLIP_PLANES);
	t->clip = (GLboolean *) crCalloc (sizeof (GLboolean) * CR_MAX_CLIP_PLANES);
	for (i = 0; i < CR_MAX_CLIP_PLANES; i++) 
	{
		t->clipPlane[i].x = 0.0f;
		t->clipPlane[i].y = 0.0f;
		t->clipPlane[i].z = 0.0f;
		t->clipPlane[i].w = 0.0f;
		t->clip[i] = GL_FALSE;
	}
	RESET(tb->clipPlane, ctx->bitid);

#ifdef CR_OPENGL_VERSION_1_2
	t->rescaleNormals = GL_FALSE;
#endif
#ifdef CR_IBM_rasterpos_clip
	t->rasterPositionUnclipped = GL_FALSE;
#endif

	t->modelViewProjectionValid = 0;

	RESET(tb->dirty, ctx->bitid);
}

/*
 * Compute the project of the modelview and projection matrices
 */
void crStateTransformUpdateTransform (CRTransformState *t) 
{
	const CRmatrix *m1 = t->projectionStack.top;
	const CRmatrix *m2 = t->modelViewStack.top;
	CRmatrix *m = &(t->modelViewProjection);
	/* I really hate this. */
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
	t->modelViewProjectionValid = 1;
}

void crStateTransformXformPoint(CRTransformState *t, GLvectorf *p) 
{
	/* XXX is this flag really needed?  We may be covering a bug elsewhere */
	if (!t->modelViewProjectionValid)
		crStateTransformUpdateTransform(t);

	crStateTransformXformPointMatrixf(&(t->modelViewProjection), p);
}

void crStateTransformXformPointMatrixf(const CRmatrix *m, GLvectorf *p) 
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

void crStateTransformXformPointMatrixd(const CRmatrix *m, GLvectord *p)
{
	GLdouble x = p->x;
	GLdouble y = p->y;
	GLdouble z = p->z;
	GLdouble w = p->w;

	p->x = (GLdouble) (m->m00*x + m->m10*y + m->m20*z + m->m30*w);
	p->y = (GLdouble) (m->m01*x + m->m11*y + m->m21*z + m->m31*w);
	p->z = (GLdouble) (m->m02*x + m->m12*y + m->m22*z + m->m32*w);
	p->w = (GLdouble) (m->m03*x + m->m13*y + m->m23*z + m->m33*w);
}

void crStateTransformInvertTransposeMatrix(CRmatrix *inv, const CRmatrix *mat)
{
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


void STATE_APIENTRY crStateClipPlane (GLenum plane, const GLdouble *equation)
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &g->transform;
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	GLvectord e;
	unsigned int i;
	CRmatrix inv;

	e.x = equation[0];
	e.y = equation[1];
	e.z = equation[2];
	e.w = equation[3];

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "ClipPlane called in begin/end");
		return;
	}

	FLUSH();

	i = plane - GL_CLIP_PLANE0;
	if (i >= g->limits.maxClipPlanes)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					 "ClipPlane called with bad enumerant: %d", plane);
		return;
	}

	crStateTransformInvertTransposeMatrix(&inv, t->modelViewStack.top);
	crStateTransformXformPointMatrixd (&inv, &e);
	t->clipPlane[i] = e;
	DIRTY(tb->clipPlane, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateMatrixMode(GLenum e) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRTextureState *tex = &(g->texture);
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
			t->currentStack = &(t->modelViewStack);
			t->matrixMode = GL_MODELVIEW;
			tb->currentMatrix = tb->modelviewMatrix;
			break;
		case GL_PROJECTION:
			t->currentStack = &(t->projectionStack);
			t->matrixMode = GL_PROJECTION;
			tb->currentMatrix = tb->projectionMatrix;
			break;
		case GL_TEXTURE:
			t->currentStack = &(t->textureStack[tex->curTextureUnit]);
			t->matrixMode = GL_TEXTURE;
			tb->currentMatrix = tb->textureMatrix;
			break;
		case GL_COLOR:
			t->currentStack = &(t->colorStack);
			t->matrixMode = GL_COLOR;
			tb->currentMatrix = tb->colorMatrix;
			break;
		case GL_MATRIX0_NV:
		case GL_MATRIX1_NV:
		case GL_MATRIX2_NV:
		case GL_MATRIX3_NV:
		case GL_MATRIX4_NV:
		case GL_MATRIX5_NV:
		case GL_MATRIX6_NV:
		case GL_MATRIX7_NV:
			if (g->extensions.NV_vertex_program) {
				const GLint i = e - GL_MATRIX0_NV;
				t->currentStack = &(t->programStack[i]);
				t->matrixMode = e;
				tb->currentMatrix = tb->programMatrix;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid matrix mode: %d", e);
				return;
			}
			break;
		case GL_MATRIX0_ARB:
		case GL_MATRIX1_ARB:
		case GL_MATRIX2_ARB:
		case GL_MATRIX3_ARB:
		case GL_MATRIX4_ARB:
		case GL_MATRIX5_ARB:
		case GL_MATRIX6_ARB:
		case GL_MATRIX7_ARB:
			/* Note: the NV and ARB program matrices are the same, but
			 * use different enumerants.
			 */
			if (g->extensions.ARB_vertex_program) {
				const GLint i = e - GL_MATRIX0_ARB;
				t->currentStack = &(t->programStack[i]);
				t->matrixMode = e;
				tb->currentMatrix = tb->programMatrix;
			}
			else {
				 crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid matrix mode: %d", e);
				 return;
			}
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid matrix mode: %d", e);
			return;
	}
	DIRTY(tb->matrixMode, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
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

	*(t->currentStack->top) = identity_matrix;
	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
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

	if (t->currentStack->depth == 0)
	{
		crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "PopMatrix of empty stack.");
		return;
	}

	if (t->currentStack->depth < 0 ||
		 t->currentStack->depth >= t->currentStack->maxDepth ) 
	{
		crError( "Invalid depth in PopMatrix: %d", t->currentStack->depth );
	}

	t->currentStack->depth--;
	t->currentStack->top = t->currentStack->stack + t->currentStack->depth;

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
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

	if (t->currentStack->depth + 1 == t->currentStack->maxDepth)
	{
		crStateError(__LINE__, __FILE__, GL_STACK_OVERFLOW, "PushMatrix pass the end of allocated stack");
		return;
	}

	if (t->currentStack->depth < 0 ||
		t->currentStack->depth >= t->currentStack->maxDepth)
	{
		crError( "Bogus depth in PushMatrix: %d", t->currentStack->depth );
	}

	/* Perform the copy */
	*(t->currentStack->top + 1) = *(t->currentStack->top);

	/* Move the stack pointer */
	t->currentStack->depth++;
	t->currentStack->top = t->currentStack->stack + t->currentStack->depth;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}


void STATE_APIENTRY crStateLoadMatrixf(const GLfloat *m1) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	CRmatrix *m;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "LoadMatrixf called in begin/end");
		return;
	}

	FLUSH();

	m = t->currentStack->top;
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

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);

}

void STATE_APIENTRY crStateLoadMatrixd(const GLdouble *m1) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	CRmatrix *m;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "LoadMatrixd called in begin/end");
		return;
	}

	FLUSH();

	m = t->currentStack->top;
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

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateLoadTransposeMatrixfARB(const GLfloat *m1) 
{
   GLfloat tm[16];
   if (!m1) return;
   _math_transposef(tm, m1);
   crStateLoadMatrixf(tm);
}

void STATE_APIENTRY crStateLoadTransposeMatrixdARB(const GLdouble *m1) 
{
   GLdouble tm[16];
   if (!m1) return;
   _math_transposed(tm, m1);
   crStateLoadMatrixd(tm);
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
	CRmatrix *m = t->currentStack->top;
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

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateMultMatrixd(const GLdouble *m1) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	CRmatrix *m = t->currentStack->top;
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

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateMultTransposeMatrixfARB(const GLfloat *m1) 
{
   GLfloat tm[16];
   if (!m1) return;
   _math_transposef(tm, m1);
   crStateMultMatrixf(tm);
}

void STATE_APIENTRY crStateMultTransposeMatrixdARB(const GLdouble *m1) 
{
   GLdouble tm[16];
   if (!m1) return;
   _math_transposed(tm, m1);
   crStateMultMatrixd(tm);
}

void STATE_APIENTRY crStateTranslatef(GLfloat x_arg, GLfloat y_arg, GLfloat z_arg) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	CRmatrix *m = t->currentStack->top;
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

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}


void STATE_APIENTRY crStateTranslated(GLdouble x_arg, GLdouble y_arg, GLdouble z_arg) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	CRmatrix *m = t->currentStack->top;
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

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
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

	/* Begin/end Checking and flushing will be done by MultMatrix. */

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

	/* Begin/end Checking and flushing will be done by MultMatrix. */


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
	CRmatrix *m = t->currentStack->top;
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

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateScaled (GLdouble x_arg, GLdouble y_arg, GLdouble z_arg) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &(g->transform);
	CRStateBits *sb = GetCurrentBits();
	CRTransformBits *tb = &(sb->transform);
	CRmatrix *m = t->currentStack->top;
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

	t->modelViewProjectionValid = 0;

	DIRTY(tb->currentMatrix, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateFrustum(GLdouble left, GLdouble right,
																	 GLdouble bottom, GLdouble top, 
																	 GLdouble zNear, GLdouble zFar)
{
	GLdouble m[16];

	/* Begin/end Checking and flushing will be done by MultMatrix. */

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

void STATE_APIENTRY crStateOrtho(GLdouble left, GLdouble right,
																 GLdouble bottom, GLdouble top,
																 GLdouble znear, GLdouble zfar)
{

	GLdouble m[16];	

	/* Begin/end Checking and flushing will be done by MultMatrix. */

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

void STATE_APIENTRY crStateGetClipPlane(GLenum plane, GLdouble *equation) 
{
	CRContext *g = GetCurrentContext();
	CRTransformState *t = &g->transform;
	unsigned int i;
	
	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"glGetClipPlane called in begin/end");
		return;
	}

	i = plane - GL_CLIP_PLANE0;
	if (i >= g->limits.maxClipPlanes)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
			"GetClipPlane called with bad enumerant: %d", plane);
		return;
	}

	equation[0] = t->clipPlane[i].x;
	equation[1] = t->clipPlane[i].y;
	equation[2] = t->clipPlane[i].z;
	equation[3] = t->clipPlane[i].w;
}

void crStateTransformSwitch( CRTransformBits *t, CRbitvalue *bitID, 
														 CRContext *fromCtx, CRContext *toCtx )
{
	const GLuint maxTextureUnits = toCtx->limits.maxTextureUnits;
	CRTransformState *from = &(fromCtx->transform);
	CRTransformState *to = &(toCtx->transform);
	GLuint i, j;
	unsigned int checktex = 0;
	CRbitvalue nbitID[CR_MAX_BITARRAY];

	for (j=0;j<CR_MAX_BITARRAY;j++)
		nbitID[j] = ~bitID[j];

	if (CHECKDIRTY(t->enable, bitID))
	{
		glAble able[2];
		able[0] = diff_api.Disable;
		able[1] = diff_api.Enable;
		if (from->normalize != to->normalize) {
			if (to->normalize == GL_TRUE)
				diff_api.Enable(GL_NORMALIZE);
			else
				diff_api.Disable(GL_NORMALIZE);
			FILLDIRTY(t->enable);
			FILLDIRTY(t->dirty);
		}
#ifdef CR_OPENGL_VERSION_1_2
		if (from->rescaleNormals != to->rescaleNormals)
		{
			able[to->rescaleNormals](GL_RESCALE_NORMAL);
			FILLDIRTY(t->enable);
			FILLDIRTY(t->dirty);
		}
#else
		(void) able;
#endif
#ifdef CR_IBM_rasterpos_clip
		if (from->rasterPositionUnclipped != to->rasterPositionUnclipped)
		{
			able[to->rasterPositionUnclipped](GL_RASTER_POSITION_UNCLIPPED_IBM);
			FILLDIRTY(t->enable);
			FILLDIRTY(t->dirty);
		}
#endif
		CLEARDIRTY(t->enable, nbitID);
	}

	if (CHECKDIRTY(t->clipPlane, bitID)) {
		diff_api.MatrixMode(GL_MODELVIEW);
		diff_api.PushMatrix();
		diff_api.LoadIdentity();
		for (i=0; i<CR_MAX_CLIP_PLANES; i++) {
			if (from->clipPlane[i].x != to->clipPlane[i].x ||
				from->clipPlane[i].y != to->clipPlane[i].y ||
				from->clipPlane[i].z != to->clipPlane[i].z ||
				from->clipPlane[i].w != to->clipPlane[i].w) {

				GLdouble cp[4];
				cp[0] = to->clipPlane[i].x;
				cp[1] = to->clipPlane[i].y;
				cp[2] = to->clipPlane[i].z;
				cp[3] = to->clipPlane[i].w;

				diff_api.ClipPlane(GL_CLIP_PLANE0 + i, (const GLdouble *)(cp));

				FILLDIRTY(t->clipPlane);
				FILLDIRTY(t->dirty);
			}
		}
		diff_api.PopMatrix();
		CLEARDIRTY(t->clipPlane, nbitID);
	}

	/* If the matrix stack depths don't match when we're
	 * updating the context - we can update the wrong matrix
	 * and get some lovely effects!! 
	 * So we troll each depth list here and Pop & Push the matrix stack
	 * to bring it right up to date before checking the current
	 * matrix.
	 *
	 * - Alan H.
	 */
	if ( (from->modelViewStack.depth != to->modelViewStack.depth) ||
	      CHECKDIRTY(t->modelviewMatrix, bitID) )
	{
		int td = to->modelViewStack.depth;
		int fd = from->modelViewStack.depth;

		diff_api.MatrixMode(GL_MODELVIEW);

		if (fd > td)
		{
			for (i = td; i < fd; i++) 
			{
				diff_api.PopMatrix();
			}
			fd = td;
		}

		for (i = fd; i <= td; i++)
		{
			LOADMATRIX(to->modelViewStack.stack + i);
			FILLDIRTY(t->modelviewMatrix);
			FILLDIRTY(t->dirty);

			/* Don't want to push on the current matrix */
			if (i != to->modelViewStack.depth)
				diff_api.PushMatrix();
		}
		CLEARDIRTY(t->modelviewMatrix, nbitID);
	}

	if ( (from->projectionStack.depth != to->projectionStack.depth) ||
	      CHECKDIRTY(t->projectionMatrix, bitID) )
	{
		int td = to->projectionStack.depth;
		int fd = from->projectionStack.depth;

		diff_api.MatrixMode(GL_PROJECTION);

		if (fd > td)
		{
			for (i = td; i < fd; i++) 
			{
				diff_api.PopMatrix();
			}
			fd = td;
		}

		for (i = fd; i <= td; i++)
		{
			LOADMATRIX(to->projectionStack.stack + i);
			FILLDIRTY(t->projectionMatrix);
			FILLDIRTY(t->dirty);

			/* Don't want to push on the current matrix */
			if (i != to->projectionStack.depth)
				diff_api.PushMatrix();
		}
		CLEARDIRTY(t->projectionMatrix, nbitID);
	}

	for (i = 0 ; i < maxTextureUnits ; i++)
		if (from->textureStack[i].depth != to->textureStack[i].depth)
			checktex = 1;
	
	if ( checktex || CHECKDIRTY(t->textureMatrix, bitID) )
	{
		for (j = 0 ; j < maxTextureUnits ; j++)
		{
			int td = to->textureStack[j].depth;
			int fd = from->textureStack[j].depth;

			diff_api.MatrixMode(GL_TEXTURE);

			if (fd > td)
			{
				for (i = td; i < fd; i++) 
				{
					diff_api.PopMatrix();
				}
				fd = td;
			}

			for (i = fd; i <= td; i++)
			{
				diff_api.ActiveTextureARB( j + GL_TEXTURE0_ARB );
				LOADMATRIX(to->textureStack[j].stack + i);
				FILLDIRTY(t->textureMatrix);
				FILLDIRTY(t->dirty);
	
				/* Don't want to push on the current matrix */
				if (i != to->textureStack[j].depth)
					diff_api.PushMatrix();
			}
		}
		CLEARDIRTY(t->textureMatrix, nbitID);
	}

	if ( (from->colorStack.depth != to->colorStack.depth) ||
	      CHECKDIRTY(t->colorMatrix, bitID) )
	{
		int td = to->colorStack.depth;
		int fd = from->colorStack.depth;

		diff_api.MatrixMode(GL_COLOR);

		if (fd > td)
		{
			for (i = td; i < fd; i++) 
			{
				diff_api.PopMatrix();
			}
			fd = td;
		}

		for (i = fd; i <= td; i++)
		{
			LOADMATRIX(to->colorStack.stack + i);
			FILLDIRTY(t->colorMatrix);
			FILLDIRTY(t->dirty);

			/* Don't want to push on the current matrix */
			if (i != to->colorStack.depth)
				diff_api.PushMatrix();
		}
		CLEARDIRTY(t->colorMatrix, nbitID);
	}

	to->modelViewProjectionValid = 0;
	CLEARDIRTY(t->dirty, nbitID);

	/* Since we were mucking with the current matrix and texture unit above 
	 * set it to the proper value now.  
	 */
	diff_api.MatrixMode(to->matrixMode);
	diff_api.ActiveTextureARB(GL_TEXTURE0_ARB + toCtx->texture.curTextureUnit);
}

void
crStateTransformDiff( CRTransformBits *t, CRbitvalue *bitID,
											CRContext *fromCtx, CRContext *toCtx )
{
	const GLuint maxTextureUnits = toCtx->limits.maxTextureUnits;
	CRTransformState *from = &(fromCtx->transform);
	CRTransformState *to = &(toCtx->transform);
	CRTextureState *textureFrom = &(fromCtx->texture);
	GLuint i, j;
	unsigned int checktex = 0;
	CRbitvalue nbitID[CR_MAX_BITARRAY];

	for (j=0;j<CR_MAX_BITARRAY;j++)
		nbitID[j] = ~bitID[j];

	if (CHECKDIRTY(t->enable, bitID)) {
		glAble able[2];
		able[0] = diff_api.Disable;
		able[1] = diff_api.Enable;
		for (i=0; i<CR_MAX_CLIP_PLANES; i++) {
			if (from->clip[i] != to->clip[i]) {
				if (to->clip[i] == GL_TRUE)
					diff_api.Enable(GL_CLIP_PLANE0 + i);
				else
					diff_api.Disable(GL_CLIP_PLANE0 + i);
				from->clip[i] = to->clip[i];
			}
		}
		if (from->normalize != to->normalize) {
			if (to->normalize == GL_TRUE)
				diff_api.Enable(GL_NORMALIZE);
			else
				diff_api.Disable(GL_NORMALIZE);
			from->normalize = to->normalize;
		}
#ifdef CR_OPENGL_VERSION_1_2
		if (from->rescaleNormals != to->rescaleNormals) {
			able[to->rescaleNormals](GL_RESCALE_NORMAL);
			from->rescaleNormals = to->rescaleNormals;
		}
#else
		(void) able;
#endif
#ifdef CR_IBM_rasterpos_clip
		if (from->rasterPositionUnclipped != to->rasterPositionUnclipped) {
			able[to->rasterPositionUnclipped](GL_RASTER_POSITION_UNCLIPPED_IBM);
			from->rasterPositionUnclipped = to->rasterPositionUnclipped;
		}
#endif

		CLEARDIRTY(t->enable, nbitID);
	}

	if (CHECKDIRTY(t->clipPlane, bitID)) {
		if (from->matrixMode != GL_MODELVIEW) {
			diff_api.MatrixMode(GL_MODELVIEW);
			from->matrixMode = GL_MODELVIEW;
		}
		diff_api.PushMatrix();
		diff_api.LoadIdentity();
		for (i=0; i<CR_MAX_CLIP_PLANES; i++) {
			if (from->clipPlane[i].x != to->clipPlane[i].x ||
				from->clipPlane[i].y != to->clipPlane[i].y ||
				from->clipPlane[i].z != to->clipPlane[i].z ||
				from->clipPlane[i].w != to->clipPlane[i].w) {
				
				GLdouble cp[4];
				cp[0] = to->clipPlane[i].x;
				cp[1] = to->clipPlane[i].y;
				cp[2] = to->clipPlane[i].z;
				cp[3] = to->clipPlane[i].w;

				diff_api.ClipPlane(GL_CLIP_PLANE0 + i, (const GLdouble *)(cp));
				from->clipPlane[i] = to->clipPlane[i];
			}
		}
		diff_api.PopMatrix();
		CLEARDIRTY(t->clipPlane, nbitID);
	}
	
	/* If the matrix stack depths don't match when we're
	 * updating the context - we can update the wrong matrix
	 * and get some lovely effects!! 
	 * So we troll each depth list here and Pop & Push the matrix stack
	 * to bring it right up to date before checking the current
	 * matrix.
	 *
	 * - Alan H.
	 */
	if ( (from->modelViewStack.depth != to->modelViewStack.depth) ||
	      CHECKDIRTY(t->modelviewMatrix, bitID) )
	{
		if (from->matrixMode != GL_MODELVIEW) {
			diff_api.MatrixMode(GL_MODELVIEW);
			from->matrixMode = GL_MODELVIEW;
		}

		if (from->modelViewStack.depth > to->modelViewStack.depth) 
		{
			for (i = to->modelViewStack.depth; i < from->modelViewStack.depth; i++) 
			{
				diff_api.PopMatrix();
			}

			from->modelViewStack.depth = to->modelViewStack.depth;
		}

		for (i = from->modelViewStack.depth; i <= to->modelViewStack.depth; i++)
		{
			LOADMATRIX(to->modelViewStack.stack + i);
			crMemcpy(from->modelViewStack.stack + i, to->modelViewStack.stack + i,
							 sizeof(CRmatrix));

			/* Don't want to push on the current matrix */
			if (i != to->modelViewStack.depth)
				diff_api.PushMatrix();
		}
		from->modelViewStack.depth = to->modelViewStack.depth;

		CLEARDIRTY(t->modelviewMatrix, nbitID);
	}

	if ( (from->projectionStack.depth != to->projectionStack.depth) ||
	      CHECKDIRTY(t->projectionMatrix, bitID) )
	{
		if (from->matrixMode != GL_PROJECTION) {
			diff_api.MatrixMode(GL_PROJECTION);
			from->matrixMode = GL_PROJECTION;
		}

		if (from->projectionStack.depth > to->projectionStack.depth) 
		{
			for (i = to->projectionStack.depth; i < from->projectionStack.depth; i++) 
			{
				diff_api.PopMatrix();
			}

			from->projectionStack.depth = to->projectionStack.depth;
		}

		for (i = from->projectionStack.depth; i <= to->projectionStack.depth; i++)
		{
			LOADMATRIX(to->projectionStack.stack + i);
			crMemcpy(from->projectionStack.stack + i, to->projectionStack.stack + i,
							 sizeof(CRmatrix));

			/* Don't want to push on the current matrix */
			if (i != to->projectionStack.depth)
				diff_api.PushMatrix();
		}
		from->projectionStack.depth = to->projectionStack.depth;

		CLEARDIRTY(t->projectionMatrix, nbitID);
	}

	for (i = 0 ; i < maxTextureUnits ; i++)
		if (from->textureStack[i].depth != to->textureStack[i].depth)
			checktex = 1;
	
	if ( checktex || CHECKDIRTY(t->textureMatrix, bitID) )
	{
		for (j = 0 ; j < maxTextureUnits; j++)
		{
			if (from->matrixMode != GL_TEXTURE) {
				diff_api.MatrixMode(GL_TEXTURE);
				from->matrixMode = GL_TEXTURE;
			}

			if (from->textureStack[j].depth > to->textureStack[j].depth) 
			{
				for (i = to->textureStack[j].depth; i < from->textureStack[j].depth; i++) 
				{
					diff_api.PopMatrix();
				}
	
				from->textureStack[j].depth = to->textureStack[j].depth;
			}

			for (i = from->textureStack[j].depth; i <= to->textureStack[j].depth; i++)
			{
				if (textureFrom->curTextureUnit != j) {
					diff_api.ActiveTextureARB( j + GL_TEXTURE0_ARB );
					textureFrom->curTextureUnit = j;
				}
				LOADMATRIX(to->textureStack[j].stack + i);
				crMemcpy(from->textureStack[j].stack + i,
								 to->textureStack[j].stack + i, sizeof(CRmatrix));

				/* Don't want to push on the current matrix */
				if (i != to->textureStack[j].depth)
					diff_api.PushMatrix();
			}
			from->textureStack[j].depth = to->textureStack[j].depth;
		}
		CLEARDIRTY(t->textureMatrix, nbitID);
	}

	if ( (from->colorStack.depth != to->colorStack.depth) ||
	      CHECKDIRTY(t->colorMatrix, bitID) )
	{
		if (from->matrixMode != GL_COLOR) {
			diff_api.MatrixMode(GL_COLOR);
			from->matrixMode = GL_COLOR;
		}

		if (from->colorStack.depth > to->colorStack.depth) 
		{
			for (i = to->colorStack.depth; i < from->colorStack.depth; i++) 
			{
				diff_api.PopMatrix();
			}

			from->colorStack.depth = to->colorStack.depth;
		}

		for (i = to->colorStack.depth; i <= to->colorStack.depth; i++)
		{
			if (from->matrixMode != GL_COLOR) {
				diff_api.MatrixMode(GL_COLOR);
				from->matrixMode = GL_COLOR;
			}
			LOADMATRIX(to->colorStack.stack + i);
			crMemcpy(from->colorStack.stack + i, to->colorStack.stack + i, sizeof(CRmatrix));

			/* Don't want to push on the current matrix */
			if (i != to->colorStack.depth)
				diff_api.PushMatrix();
		}
		from->colorStack.depth = to->colorStack.depth;

		CLEARDIRTY(t->colorMatrix, nbitID);
	}

	to->modelViewProjectionValid = 0;
	CLEARDIRTY(t->dirty, nbitID);

	/* update MatrixMode now */
	if (from->matrixMode != to->matrixMode) {
		diff_api.MatrixMode(to->matrixMode);
		from->matrixMode = to->matrixMode;
	}
}
